//
// Created by robert-grigoryan on 5/30/24.
//
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>  // For sockaddr_in, sockaddr_in6
#include <sys/socket.h>  // For socket functions
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <iomanip>

#include "ErrorCritical.h"
#include "Logger.h"
#include "MessageBUSY.h"
#include "MessageDEAL.h"
#include "MessageIAM.h"
#include "Server.h"
#include "Utilities.h"

std::variant<std::unique_ptr<Server>, Error> Server::Create(
    const ServerConfig& config) {
  auto server = std::unique_ptr<Server>(new Server(config));
  if (const std::optional<Error> ret =
          server->setupPollfds(config.getNetworkingConfig().getPortConfig());
      ret.has_value()) {
    return ret.value();
  }

  auto ret = Utilities::getAddressFromFd(server->pollfds[kListeningIndex].fd);
  if (std::holds_alternative<Error>(ret)) {
    return std::get<Error>(ret);
  }

  const auto address = std::get<sockaddr_in6>(ret);
  const auto AddressStr = Utilities::getStringFromAddress(address);
  if (std::holds_alternative<Error>(AddressStr)) {
    return std::get<Error>(AddressStr);
  }

  server->setPort(address);
  if (auto err = server->setAddressStr(address); err.has_value()) {
    return err.value();
  }

  Logger::log("Server is listening on port " + std::to_string(server->port));

  return server;
}

Server::Server(const ServerConfig& _config)
    : port(-1),
      dealsToPlay(_config.getDeals()),
      isGameInitiated(false),
      nCandidates(0),
      nCurrentPlayers(0),
      // There are 5 connections reserved for 4 players and a listening socket.
      kMaxNCandidates(kNConnections - 5),
      kTimeout(_config.getNetworkingConfig().getTimeout()),
      pollfds() {}

Server::~Server() {
  for (auto& [fd, events, revents] : pollfds) {
    if (fd != -1) {
      close(fd);
    }
  }
}

std::optional<Error> Server::initGame() {
  std::optional<Error> err = std::nullopt;

  if (err = updateEvents(); err.has_value()) {
    return err;
  }

  for (Seat seat : Seat::getAllSeats()) {
    const int pollfdIndex = 1 + seat.getRank();
    sendMessage(pollfdIndex,
                *MessageDEAL::FromConfig(dealsToPlay[0], seat.getPosition()));
  }

  game.playNewDeal(dealsToPlay[0]);

  isGameInitiated = true;
  Logger::log("Game initiated.");
  return std::nullopt;
}

std::optional<Error> Server::playGame() {
  while (!dealsToPlay.empty()) {
    if (std::optional<Error> err = updateEvents(); err.has_value()) {
      return err.value();
    }

    if (auto err = handlePlayerEvents(); err.has_value()) {
      return err;
    }
  }

  return std::nullopt;
}

std::optional<Error> Server::setupPollfds(const PortConfig port) {
  Logger::log("Setup server with port " + port.toString());
  const std::variant<int, Error> listeningfd = getListeningSocket(port);
  if (std::holds_alternative<Error>(listeningfd)) {
    return std::get<Error>(listeningfd);
  }

  for (auto& [fd, event, revent] : pollfds) {
    fd = -1;
    event = 0;
    revent = 0;
  }

  pollfds[kListeningIndex].fd = std::get<int>(listeningfd);

  return std::nullopt;
}

void Server::setupListeningPollfdEvents() {
  pollfds[kListeningIndex].events = nCandidates < kMaxNCandidates ? POLLIN : 0;
}

std::variant<int, Error> Server::getListeningSocket(const PortConfig port) {
  std::optional<Error> ret = std::nullopt;
  int socketfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (socketfd == -1) {
    return Error("ERROR: socket - " + std::string(strerror(errno)));
  }

  if (ret = acceptBothIPv4AndIPv6(socketfd); ret.has_value()) {
    close(socketfd);
    return ret.value();
  }

  if (ret = bindSocket(socketfd, port); ret.has_value()) {
    close(socketfd);
    return ret.value();
  }

  if (listen(socketfd, kListenBacklog) == -1) {
    close(socketfd);
    return Error("ERROR: listen - " + std::string(strerror(errno)));
  }

  return socketfd;
}

std::optional<Error> Server::acceptBothIPv4AndIPv6(const int socketfd) {
  constexpr int opt = 0;
  if (setsockopt(socketfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) ==
      -1) {
    return Error("ERROR: setsockopt - " + std::string(strerror(errno)));
  }
  return std::nullopt;
}

std::optional<Error> Server::bindSocket(const int socketfd,
                                        const PortConfig port) {
  sockaddr_in6 address = {};
  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons(port.getPort().value_or(0));

  if (bind(socketfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
      -1) {
    return Error("ERROR: bind - " + std::string(strerror(errno)));
  }

  return std::nullopt;
}

std::optional<int> Server::getTimeout() {
  const time_t currentTime = time(nullptr);
  std::optional<int> timeout_min = std::nullopt;

  for (std::optional<time_t>& time : waitingSince) {
    if (!time.has_value()) {
      continue;
    }
    if (time.value() <= currentTime - kTimeout) {
      Logger::log("WARNING: Invalid time value " +
                  std::to_string(time.value()));
      continue;
    }

    const int timeout = static_cast<int>(time.value() + kTimeout - currentTime);
    if (!timeout_min.has_value() || timeout < timeout_min.value()) {
      timeout_min = timeout;
    }
  }

  if (timeout_min.has_value()) {
    timeout_min = timeout_min.value() * 1000;
  }

  return timeout_min;
}

void Server::setPort(const sockaddr_in6& address) {
  port = ntohs(address.sin6_port);
}

std::optional<Error> Server::setAddressStr(sockaddr_in6 addr) {
  char ip_str[INET6_ADDRSTRLEN];  // Buffer to hold the IPv6 string

  // Convert IPv6 address to string
  if (inet_ntop(AF_INET6, &addr.sin6_addr, ip_str, sizeof(ip_str)) == nullptr) {
    return ErrorCritical("ERROR: inet_ntop - " + std::string(strerror(errno)));
  }

  // Convert port from network byte order to host byte order
  uint16_t port = ntohs(addr.sin6_port);

  // Format the string as <ipv6>:<port>
  std::ostringstream oss;
  oss << ip_str << ":" << port;
  address = oss.str();
  return std::nullopt;
}

std::variant<std::unique_ptr<Message>, Error> Server::receiveMessage(
    int pollfdIndex) const {
  auto ret = connections.at(pollfdIndex)->popMessage();
  if (std::holds_alternative<std::unique_ptr<Message>>(ret)) {
    reportReceivedMessage(pollfdIndex,
                          *std::get<std::unique_ptr<Message>>(ret));
  }

  return ret;
}

void Server::sendMessage(const int pollfdIndex, const Message& message) const {
  connections.at(pollfdIndex)->sendMessage(message);
  reportSentMessage(pollfdIndex, message);
}

std::optional<Error> Server::sendTrickToCurrentPlayer() const {
  const int firstPlayerPollfdIndex =
      1 + Seat(dealsToPlay[0].getFirstPlayer()).getRank();
  sendMessage(firstPlayerPollfdIndex, *MessageTrick::Create(1, {}));
  if (auto err = connections.at(firstPlayerPollfdIndex)->beginTimeout();
      err.has_value()) {
    return err;
  }
  return std::nullopt;
}

std::optional<Error> Server::playCard(const MessageTrick& trick) {
  if (trick.getTrickNumber() != game.getCurrentTrickNumber()) {
    return Error("playCard - Invalid trick number.");
  }

  return game.playCard(trick.getCards().back());
}

std::optional<int> Server::getPollfdForCandidate() const {
  std::optional<int> freeSpot = std::nullopt;
  for (int i = 5; i < kNConnections; ++i) {
    if (pollfds[i].fd == -1) {
      freeSpot = i;
      break;
    }
  }
  return freeSpot;
}

std::optional<Error> Server::addCandidate(int pollfdIndex) {
  if (pollfds[pollfdIndex].fd != -1 || connections.contains(pollfdIndex)) {
    return Error("ERROR: addCandidate - pollfdIndex is already taken.");
  }

  // This connection object will now manage the pollfd.
  auto ret =
      Connection::Accept(&pollfds[pollfdIndex], waitingSince[pollfdIndex],
                         pollfds[kListeningIndex].fd);
  if (std::holds_alternative<Error>(ret)) {
    return std::get<Error>(ret);
  }
  std::unique_ptr<Connection> candidate =
      std::move(std::get<std::unique_ptr<Connection>>(ret));
  if (auto error = candidate->beginTimeout(); error.has_value()) {
    return error.value();
  }
  connections.emplace(pollfdIndex, std::move(candidate));
  Logger::log("Waiting for IAM from candidate " +
              std::to_string(pollfds[pollfdIndex].fd) + ".");
  return std::nullopt;
}

void Server::rejectCandidate(int pollfdIndex) {
  std::vector<Seat> takenSeats;
  for (Seat seat : Seat::getAllSeats()) {
    if (connections.contains(1 + seat.getRank())) {
      takenSeats.push_back(seat);
    }
  }
  sendMessage(pollfdIndex, *MessageBUSY::FromVector(takenSeats));
  rejectedCandidates.insert(pollfdIndex);
}

std::optional<Error> Server::updateCandidateBuffers(
    const int pollfdIndex) const {
  Connection& candidate = *connections.at(pollfdIndex);
  if (const auto timeSinceTimeout = candidate.getTimeSinceTimeout();
      timeSinceTimeout.has_value() && timeSinceTimeout.value() >= kTimeout) {
    return Error("Candidate " + std::to_string(pollfds[pollfdIndex].fd) +
                 " timeouted.");
  }
  if (std::optional<Error> error = candidate.updateBuffers();
      error.has_value()) {
    return error;
  }
  return std::nullopt;
}

std::optional<Error> Server::handleCandidateMessage(const int pollfdIndex) {
  auto ret = receiveMessage(pollfdIndex);
  if (std::holds_alternative<Error>(ret)) {
    return std::get<Error>(ret);
  }
  connections.at(pollfdIndex)->clearTimeout();
  const std::unique_ptr<Message> message =
      std::move(std::get<std::unique_ptr<Message>>(ret));

  const auto msgIamVariant = MessageIAM::FromPlayerMessge(message);
  if (std::holds_alternative<Error>(msgIamVariant)) {
    // Logger::log(std::get<Error>(miamVariant).getMessage());
    connections.erase(pollfdIndex);
    return std::nullopt;
  }
  const auto pos
    = std::get<std::unique_ptr<MessageIAM>>(msgIamVariant)->getPosition();
  if (const auto err = addPlayer(pos, pollfdIndex); err.has_value()) {
    Logger::log(err.value().getMessage());
    rejectCandidate(pollfdIndex);
    rejectedCandidates.insert(pollfdIndex);
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<Error> Server::handlePlayerEvents() {
  // TODO: add handling players who send messges without being asked to do so
  const Seat currentPlayer = game.getCurrentTurn();
  const int playerPollfdIndex = 1 + currentPlayer.getRank();
  const Connection& connection = *connections.at(playerPollfdIndex);

  if (connection.hasMessage()) {
    if (auto err = handlePlayerMessage(currentPlayer); err.has_value()) {
      return err;
    }
    return std::nullopt;
  }
  const auto timeSinceTimeout = connection.getTimeSinceTimeout();
  if (timeSinceTimeout.has_value() && timeSinceTimeout.value() >= kTimeout) {
    return sendTrickToCurrentPlayer();
  }

  return std::nullopt;
}

std::optional<Error> Server::handlePlayerMessage(const Seat player) {
  const int playerPollfdIndex = 1 + player.getRank();
  if (!connections.at(playerPollfdIndex)->hasMessage()) {
    return Error("handlePlayerMessage - no message.");
  }

  const auto messageVariant = receiveMessage(playerPollfdIndex);
  if (std::holds_alternative<Error>(messageVariant)) {
    // We assume there is a message.
    if (const auto err = removePlayer(player.getPosition()); err.has_value()) {
      return ErrorCritical(err.value().getMessage());
    }
  }
  const auto msgTrickVariant = MessageTrick::FromPlayerMessage(
      *std::get<std::unique_ptr<Message>>(messageVariant));
  if (std::holds_alternative<Error>(messageVariant)) {
    if (auto err = removePlayer(player.getPosition()); err.has_value()) {
      return err;
    }
  }
  // Valid message, maybe impossible to play.
  const MessageTrick& msgTrick = *std::get<std::unique_ptr<MessageTrick>>(
    msgTrickVariant);
  if (const auto error = playCard(msgTrick); error.has_value()) {
    return error;
  }

  return std::nullopt;
}

std::optional<Error> Server::callPoll() {
  clearRevents();
  setupListeningPollfdEvents();

  if (const std::optional<int> timeoutOpt = getTimeout();
      !timeoutOpt.has_value() || timeoutOpt.value() > 0) {
    const int timeout = timeoutOpt.value_or(-1);
    Logger::log("Call poll with timeout " + std::to_string(timeout));
    const int pollStatus = poll(pollfds.data(), pollfds.size(), timeout);
    Logger::log("Poll status: " + std::to_string(pollStatus));

    if (pollStatus == -1) {
      return ErrorCritical("ERROR: poll - " + std::string(strerror(errno)));
    }
  } else {
    Logger::log("Skipping poll, timeout equals zero.");
  }

  return std::nullopt;
}

std::optional<Error> Server::updatePlayers() {
  for (Seat seat : Seat::getAllSeats()) {
    int player = 1 + seat.getRank();
    if (!connections.contains(player)) {
      continue;
    }

    if (auto err = connections.at(player)->updateBuffers(); err.has_value()) {
      if (err.value().isCritical()) {
        return err;
      }
      Logger::log(err.value().getMessage());
      return removePlayer(seat.getPosition());
    }
  }

  return std::nullopt;
}

std::optional<Error> Server::updateCandidates() {
  std::optional<Error> error = std::nullopt;

  for (int id = 1 + kNPlayers; id < kNConnections; id++) {
    if (!connections.contains(id)) {
      continue;
    }

    if (error = updateCandidate(id); error.has_value()) {
      return error.value();
    }
  }

  return std::nullopt;
}

std::optional<Error> Server::updateCandidate(const int pollfdIndex) {
  std::optional<Error> err = updateCandidateBuffers(pollfdIndex);
  if (err.has_value()) {
    if (err->isCritical()) {
      return err.value();
    }
    Logger::log(err->getMessage());
    connections.erase(pollfdIndex);
    return std::nullopt;
  }

  if (!rejectedCandidates.contains(pollfdIndex)) {
    if (!connections.at(pollfdIndex)->hasMessage()) {
      return std::nullopt;
    }

    return handleCandidateMessage(pollfdIndex);
  }

  // Rejected candidate
  if (connections.at(pollfdIndex)->isOutgoingBufferEmpty()) {
    rejectedCandidates.erase(pollfdIndex);
    connections.erase(pollfdIndex);
  }

  return std::nullopt;
}

void Server::clearRevents() {
  for (auto& [fd, event, revent] : pollfds) {
    revent = 0;
  }
}

void Server::logPollEvents() {
  for (const auto& [fd, event, revent] : pollfds) {
    Logger::log(std::to_string(event));
  }
}

void Server::logPollRevents() {
  for (const auto& [fd, event, revent] : pollfds) {
    Logger::log(std::to_string(revent));
  }
}

std::optional<Error> Server::addPlayer(Seat::Position position,
                                       int pollfdIndexCandidate) {
  int pollfdIndexPlayer = 1 + Seat(position).getRank();
  if (connections.contains(pollfdIndexPlayer)) {
    return Error("Add player - place already taken");
  }

  std::unique_ptr<Connection> connection =
      std::move(connections.at(pollfdIndexCandidate));
  connections.erase(pollfdIndexCandidate);
  connection->setPollfd(&pollfds[pollfdIndexPlayer]);
  connections.emplace(pollfdIndexPlayer, std::move(connection));
  nCurrentPlayers++;

  if (isGameInitiated) {
    sendMessage(pollfdIndexPlayer,
                *MessageDEAL::FromConfig(dealsToPlay[0], position));
  }

  return std::nullopt;
}

std::optional<ErrorCritical> Server::removePlayer(Seat::Position position) {
  int pollfdIndex = 1 + Seat(position).getRank();
  if (!connections.contains(pollfdIndex)) {
    return ErrorCritical("removePlayer - No such player.");
  }
  connections.erase(pollfdIndex);
  nCurrentPlayers--;
  return std::nullopt;
}

void Server::reportReceivedMessage(const int pollfdIndex,
                                   const Message& message) const {
  std::ostringstream oss;
  oss << "[" << connections.at(pollfdIndex)->getAddressStr() << "," << address
      << "," << Utilities::getTimeStr() << "] "
      << message.getMessageWithoutSeperator();
  Logger::report(oss.str());
}

void Server::reportSentMessage(int pollfdIndex, const Message& message) const {
  std::ostringstream oss;
  oss << "[" << address << "," << connections.at(pollfdIndex)->getAddressStr()
      << "," << Utilities::getTimeStr() << "] "
      << message.getMessageWithoutSeperator();
  Logger::report(oss.str());
}

std::optional<Error> Server::run() {
  std::optional<Error> err = std::nullopt;

  if (err = initGame(); err.has_value()) {
    return err;
  }

  if (err = playGame(); err.has_value()) {
    return err;
  }

  return std::nullopt;
}

std::optional<Error> Server::updateEvents() {
  std::optional<Error> error = std::nullopt;

  do {
    if (error = callPoll(); error.has_value()) {
      return error.value();
    }

    if (error = updatePlayers(); error.has_value()) {
      return error.value();
    }
    if (error = updateListeningSocket(); error.has_value()) {
      return error.value();
    }
    if (error = updateCandidates(); error.has_value()) {
      return error.value();
    }
  } while (nCurrentPlayers < 4);

  return std::nullopt;
}

std::optional<Error> Server::updateListeningSocket() {
  if (pollfds[kListeningIndex].revents == 0) {
    return std::nullopt;
  }

  const std::optional<int> freeSpot = getPollfdForCandidate();
  if (!freeSpot.has_value()) {
    return Error("ERROR: updateListeningSocket - too many candidates.");
  }

  return addCandidate(freeSpot.value());
}
