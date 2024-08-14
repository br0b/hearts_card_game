#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <memory>
#include <sstream>

#include "MaybeError.h"
#include "Message.h"
#include "MessageBusy.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessageTaken.h"
#include "Server.h"

Server::Server(std::string separator, size_t bufferLen,
               std::chrono::milliseconds timeout)
    : maxTimeout(timeout), connectionStore(std::move(separator), bufferLen) {}

void Server::Configure(std::vector<DealConfig> deals_) {
  signal(SIGPIPE, SIG_IGN);
  deals = std::move(deals_);
  std::reverse(deals.begin(), deals.end());
}

MaybeError Server::Listen(in_port_t port, int maxTcpQueueLen) {
  return connectionStore.Listen(port, maxTcpQueueLen);
}

MaybeError Server::Run() {
  return SafeUpdate(false);
}

void Server::EnableDebug() {
  connectionStore.EnableDebug();
}

MaybeError Server::SafeUpdate(bool sendState) {
  // Gather four players.
  do {
    if (MaybeError error = Update(sendState); error.has_value()) {
      return error;
    }
  } while (playerMap.size() != 4);

  return std::nullopt;
}

MaybeError Server::Update(bool sendState) {
  time_t timeout = -1;
  time_t tmp = 0;
  MaybeError error;
  Seat seat;
  std::vector<Seat> vecBusy;
  auto now = std::chrono::system_clock::now();

  // Set time
  for (Connection &c : connections) {
    if (!c.responseDeadline.has_value()) {
      continue;
    }
    tmp = std::chrono::duration_cast<std::chrono::milliseconds>(
        c.responseDeadline.value() - now).count();
    if (tmp <= 0) {
      if (candidateMap.contains(c.socketFd)) {
        if (error = PopConnection(c.socketFd); error.has_value()) {
          return error;
        }
      } else if (playerMap.contains(c.socketFd)) {
        if (trickTimeout) {
          return std::make_unique<Error>("Server::Update",
                                         "Trick timeout hasn't been handled.");
        }
        trickTimeout = true;
      } else {
        return ErrorSocket("Server::Update");
      }
      updateArg.closed.push_back(c.socketFd);
    } else if (timeout == -1 || timeout > tmp) {
      timeout = tmp;
    }
  }

  if (error = connectionStore.Update(updateArg, timeout); error.has_value()) {
    return error;
  }
  updateRes = std::move(updateArg);
  updateArg = {{}, std::nullopt, {}};

  // Pop closed.
  for (int fd : updateRes.closed) {
    if (error = PopConnection(fd); error.has_value()) {
      return error;
    }
  }

  // New candidate
  if (updateRes.opened.has_value()) {
    if (error = PushCandidate(updateRes.opened.value()); error.has_value()) {
      return error;
    }
    connections.at(candidateMap.at(updateRes.opened.value()))
      .responseDeadline = std::chrono::system_clock::now() + maxTimeout;
  }

  // Promote candidates to players.
  for (ConnectionStore::Message msg : updateRes.msgs) {
    if (candidateMap.contains(msg.id)) {
      auto m = Message::Deserialize(std::move(msg.content));
      if (m.has_value()) {
        try {
          MessageIam &msgIam = dynamic_cast<MessageIam &>(*m.value());
          if (seatMap[msgIam.GetSeat().GetIndex()].has_value()) {
            std::ostringstream oss;
            vecBusy.clear();
            seat.Set(Seat::Value::kN);
            for (size_t p = 0 ; p < 4; p++) {
              if (seatMap[p].has_value()) {
                vecBusy.push_back(seat);
              }
              seat.CycleClockwise();
            }
            MessageBusy msgBusy;
            msgBusy.SetSeats(vecBusy);
            oss << msgBusy;
            updateArg.closed.push_back(msg.id);
            updateArg.msgs.emplace_back(msg.id, oss.str());
            if (error = PopConnection(msg.id); error.has_value()) {
              return error;
            }
          } else {
            if (error = PromoteToPlayer(msg.id, msgIam.GetSeat());
                     error.has_value()) {
              return error;
            }
            if (sendState) {
              std::ostringstream oss;
              Hand hand;
              TrickNumber trickNumber;
              MessageDeal msgDeal;
              MessageTaken msgTaken;
              msgDeal.SetFirst(deals.back().GetFirst());
              msgDeal.SetHand(
                  deals.back().GetHands()[msgIam.GetSeat().GetIndex()]);
              msgDeal.SetType(deals.back().GetType());
              oss << msgDeal;
              updateArg.msgs.emplace_back(msg.id, oss.str());
              for (size_t i = 0; i < tricks.size(); i++) {
                std::ostringstream oss;
                hand.Set(tricks[i].cards.begin(), tricks[i].cards.end());
                msgTaken.SetCards(hand);
                msgTaken.SetTaker(tricks[i].taker);
                if (error = trickNumber.Set(i + 1); error.has_value()) {
                  return error;
                }
                oss << msgTaken;
                updateArg.msgs.emplace_back(msg.id, oss.str());
              }
            }
          }
        } catch (std::bad_cast &e) {
          if (error = CloseConnection(msg.id, updateArg.closed);
              error.has_value()) {
            return error;
          }
        }
      } else if (error = CloseConnection(msg.id, updateArg.closed);
                 error.has_value()) {
        return error;
      }
    } else if (playerMap.contains(msg.id)) {
      updateArg.msgs.push_back(msg);
    } else {
      return ErrorSocket("Server::Update");
    }
  }

  return std::nullopt;
}

MaybeError Server::PushCandidate(int socketFd) {
  if (candidateMap.contains(socketFd)) {
    return ErrorSocket("Server::PushCandidate");
  }

  connections.emplace_back(socketFd, std::nullopt);
  candidateMap.emplace(socketFd, connections.size() - 1);
  return std::nullopt;
}

MaybeError Server::PromoteToPlayer(int socketFd, Seat seat) {
  if (!candidateMap.contains(socketFd) || playerMap.contains(socketFd)) {
    return ErrorSocket("Server::PromoteToPlayer");
  } else if (seatMap[seat.GetIndex()].has_value()) {
    return std::make_unique<Error>("Server::PromoteToPlayer",
                                   "Seat already taken.");
  }

  size_t index = candidateMap.at(socketFd);
  connections.at(index).responseDeadline.reset();
  candidateMap.erase(socketFd);
  seatMap[seat.GetIndex()] = index;
  playerMap.emplace(socketFd, seat);
  return std::nullopt;
}

MaybeError Server::PopConnection(int socketFd) {
  size_t index = 0;

  if (candidateMap.contains(socketFd)) {
    index = candidateMap.at(socketFd);
    if (connections.size() > 1 && index != connections.size() - 1) {
      connections[index] = connections.back();
      candidateMap.at(connections.back().socketFd) = index;
    }
    candidateMap.erase(socketFd);
  } else if (playerMap.contains(socketFd)) {
    Seat seat = playerMap.at(socketFd);
    if (!seatMap[seat.GetIndex()]) {
      return ErrorEmptySeat("Server::PopConnection");
    }
    index = seatMap[seat.GetIndex()].value();
    if (connections.size() > 1 && index != connections.size() - 1) {
      connections[index] = connections.back();
      int socketFdBack = connections.back().socketFd;
      if (!playerMap.contains(socketFdBack)) {
        return ErrorSocket("Server::PopConnection");
      }
      Seat seatBack = playerMap.at(socketFdBack);
      playerMap.at(socketFd) = seatBack;
      seatMap[seatBack.GetIndex()] = index;
    }
    playerMap.erase(socketFd);
    seatMap[seat.GetIndex()] = std::nullopt;
  } else {
    return ErrorSocket("Server::PopConnection");
  }

  connections.pop_back();
  return std::nullopt;
}

MaybeError Server::CloseConnection(int socketfd, std::vector<size_t> &closed) {
  closed.push_back(socketfd);
  return PopConnection(socketfd);
}

std::unique_ptr<Error> Server::ErrorSocket(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Socket errror.");
}

std::unique_ptr<Error> Server::ErrorEmptySeat(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Seat empty.");
}

