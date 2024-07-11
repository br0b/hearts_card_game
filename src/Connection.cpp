//
// Created by robert-grigoryan on 5/31/24.
//
#include <Logger.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <ctime>

#include "Connection.h"
#include "ErrorCritical.h"
#include "Message.h"
#include "Utilities.h"

std::variant<std::unique_ptr<Connection>, Error> Connection::Accept(
    pollfd* _pfd, std::optional<time_t>& _waitingSince, const int listeningfd) {
  auto connection =
      std::unique_ptr<Connection>(new Connection(_pfd, _waitingSince));
  auto ret = connection->acceptConnection(listeningfd);
  if (std::holds_alternative<Error>(ret)) {
    return std::get<Error>(ret);
  }
  auto address = Utilities::getStringFromAddress(std::get<sockaddr_in6>(ret));
  if (std::holds_alternative<Error>(address)) {
    return std::get<Error>(address);
  }
  connection->addressClient = std::get<std::string>(address);
  if (auto err = connection->setNonblocking(); err.has_value()) {
    return err.value();
  }

  return connection;
}

std::optional<Error> Connection::beginTimeout() const {
  if (timeSinceTimeout.has_value()) {
    return Error("Timeout already in progress");
  }

  timeSinceTimeout = time(nullptr);
  return std::nullopt;
}

std::optional<double> Connection::getTimeSinceTimeout() const {
  if (!timeSinceTimeout.has_value()) {
    return std::nullopt;
  }

  return difftime(time(nullptr), timeSinceTimeout.value());
}

void Connection::clearTimeout() const { timeSinceTimeout = std::nullopt; }

void Connection::setPollfd(pollfd* _pfd) {
  *_pfd = *pfd;
  resetPollfd();
  pfd = _pfd;
}

void Connection::resetPollfd() const {
  pfd->fd = -1;
  pfd->events = 0;
  pfd->revents = 0;
}

std::optional<Error> Connection::updateBuffers() {
  std::optional<Error> error = std::nullopt;

  if (error = updateIncoming(); error.has_value()) {
    return error;
  }

  if (error = updateOutgoing(); error.has_value()) {
    return error;
  }

  return std::nullopt;
}

bool Connection::hasMessage() const {
  return Message::containsMessage(incoming);
}

std::variant<std::unique_ptr<Message>, Error> Connection::popMessage() {
  std::optional<std::unique_ptr<Message>> message =
      Message::getFirstMessage(incoming);
  if (!message.has_value()) {
    return Error("No message to pop");
  }
  incoming = incoming.substr(message.value()->getMessage().size());
  return std::move(message.value());
}

void Connection::sendMessage(const Message& message) {
  pfd->events |= POLLOUT;
  outgoing += message.getMessage();
}

bool Connection::isOutgoingBufferEmpty() const {
  return outgoing.empty();
}

std::string Connection::getAddressStr() { return addressClient; }

std::string Connection::getServerAddressStr() { return addressServer; }

Connection::Connection(pollfd* _pfd, std::optional<time_t>& _timeSinceTimeout)
    : pfd(_pfd), timeSinceTimeout(_timeSinceTimeout) {
  *pfd = {-1, POLLIN, 0};
  timeSinceTimeout = std::nullopt;
  buffer = new char[kBufferSize];
}

std::variant<sockaddr_in6, Error> Connection::acceptConnection(
    const int listeningfd) const {
  sockaddr_in6 address{};
  socklen_t socklen = sizeof(address);
  const int clientfd = accept(listeningfd,
                              reinterpret_cast<sockaddr*>(&address),
                              &socklen);

  if (clientfd < 0) {
    return ErrorCritical("ERROR: accept - " + std::string(strerror(errno)));
  }

  pfd->fd = clientfd;
  return address;
}

std::optional<Error> Connection::setNonblocking() const {
  if (fcntl(pfd->fd, F_SETFL, O_NONBLOCK) < 0) {
    return ErrorCritical("ERROR: fcntl - " + std::string(strerror(errno)));
  }

  return std::nullopt;
}

std::optional<Error> Connection::updateIncoming() {
  if (pfd->revents & POLLERR) {
    return ErrorCritical("ERROR: POLLERR - " + std::to_string(errno));
  }
  if (!(pfd->revents & POLLIN)) {
    return std::nullopt;
  }

  const ssize_t ret = read(pfd->fd, buffer, kBufferSize);
  if (ret < 0) {
    return ErrorCritical("ERROR: read - " + std::string(strerror(errno)));
  }
  if (ret == 0) {
    return Error("Connection " + std::to_string(pfd->fd) + " disconnected");
  }

  incoming += std::string(buffer, ret);
  return std::nullopt;
}

std::optional<Error> Connection::updateOutgoing() {
  if (pfd->revents & POLLERR) {
    return ErrorCritical("ERROR: POLLERR - " + std::to_string(errno));
  }
  if (!(pfd->revents & POLLOUT)) {
    return std::nullopt;
  }
  Logger::log("Outgoing: " + outgoing);

  const size_t contentSize = std::min(outgoing.size(), kBufferSize);
  outgoing.copy(buffer, contentSize);

  const ssize_t ret = write(pfd->fd, buffer, contentSize);
  if (ret <= 0) {
    return ErrorCritical("ERROR: write - " + std::string(strerror(errno)));
  }

  outgoing = outgoing.substr(ret);
  if (outgoing.empty()) {
    pfd->events &= ~POLLOUT;
  }
  return std::nullopt;
}

Connection::~Connection() {
  Logger::log("Connection " + std::to_string(pfd->fd) + " deleted.");
  close(pfd->fd);
  resetPollfd();
  clearTimeout();
  delete[] buffer;
}
