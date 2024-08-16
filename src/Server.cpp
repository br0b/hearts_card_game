#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <memory>
#include <stdexcept>

#include "ConnectionStore.h"
#include "MaybeError.h"
#include "Message.h"
#include "MessageBusy.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessagePoints.h"
#include "MessageTaken.h"
#include "MessageTrick.h"
#include "MessageWrong.h"
#include "Server.h"

Server::Server(std::string separator, size_t bufferLen,
               std::chrono::milliseconds timeout)
    : timeout(timeout), connectionStore(std::move(separator), bufferLen) {}

void Server::Configure(std::vector<DealConfig> deals_) {
  signal(SIGPIPE, SIG_IGN);
  deals = std::move(deals_);
  std::reverse(deals.begin(), deals.end());
}

MaybeError Server::Listen(in_port_t port, int maxTcpQueueLen) {
  return connectionStore.Listen(port, maxTcpQueueLen);
}

MaybeError Server::Run() {
  if (deals.empty()) {
    return std::nullopt;
  }

  MaybeError error;
  MessageDeal msgDeal;

  // Initate the game.

  // Gather the four players.
  if (error = SafeUpdate(false); error.has_value()) {
    return error;
  }

  if (error = NewDeal(); error.has_value()) {
    return error;
  }

  if (error = SendTrick(); error.has_value()) {
    return error;
  }

  if (error = SafeUpdate(false); error.has_value()) {
    return error;
  }

  while (!deals.empty()) {
    if (error = HandlePlayerMessages(); error.has_value()) {
      return error;
    }

    if (error = SafeUpdate(true); error.has_value()) {
      return error; 
    }
  }

  return connectionStore.Close();
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
  } while (nPlayers != 4);

  return std::nullopt;
}

MaybeError Server::Update(bool sendState) {
  std::optional<std::chrono::milliseconds> earliestTimeout;
  MaybeError error;
  Seat seat;
  std::vector<Seat> vecBusy;

  // Set timeout
  std::vector<int> overdueCandidates;
  for (Connection &c : connections) {
    const auto &deadline = c.responseDeadline;
    if (!deadline.has_value()) {
      continue;
    }
    if (deadline->IsOverdue()) {
      if (candidateMap.contains(c.socketFd)) {
        overdueCandidates.push_back(c.socketFd);
      } else if (playerMap.contains(c.socketFd)) {
        return std::make_unique<Error>("Server::Update",
                                       "Unhandled player timeout.");
      } else {
        return ErrorSocket("Server::Update");
      }
    } else if (!earliestTimeout.has_value()
               || deadline->GetTimeLeft() < *earliestTimeout) {
      earliestTimeout = deadline->GetTimeLeft();
    }
  }

  // Close overdue candidates.
  for (int fd : overdueCandidates) {
    if (error = CloseConnection(fd); error.has_value()) {
      return error;
    }
  }

  if (error = connectionStore.Update(updateArg, earliestTimeout);
      error.has_value()) {
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
  }

  // Promote candidates to players.
  for (const auto &msg : updateRes.msgs) {
    if (!candidateMap.contains(msg.id)) {
      continue;
    }
    auto m = Message::Deserialize(std::move(msg.content));
    if (m != nullptr) {
      try {
        MessageIam &msgIam = dynamic_cast<MessageIam &>(*m);
        if (seatMap.at(msgIam.GetSeat().GetIndex()).has_value()) {
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
          updateArg.msgs.emplace_back(msg.id, msgBusy.Str());
          if (error = CloseConnection(msg.id); error.has_value()) {
            return error;
          }
        } else {
          if (error = PromoteToPlayer(msg.id, msgIam.GetSeat());
                   error.has_value()) {
            return error;
          }
          if (sendState) {
            Hand hand;
            TrickNumber trickNumber;
            MessageDeal msgDeal;
            MessageTaken msgTaken;
            msgDeal.SetFirst(deals.back().GetFirst());
            msgDeal.SetHand(
                deals.back().GetHands()[msgIam.GetSeat().GetIndex()]);
            msgDeal.SetType(deals.back().GetType());
            updateArg.msgs.emplace_back(msg.id, msgDeal.Str());
            for (size_t i = 0; i < tricks.size(); i++) {
              hand.Set(tricks[i].cards.begin(), tricks[i].cards.end());
              msgTaken.SetCards(hand);
              msgTaken.SetTaker(tricks[i].taker);
              if (error = trickNumber.Set(i + 1); error.has_value()) {
                return error;
              }
              updateArg.msgs.emplace_back(msg.id, msgTaken.Str());
            }
          }
        }
      } catch (std::bad_cast &e) {
        if (error = CloseConnection(msg.id);
            error.has_value()) {
          return error;
        }
      }
    } else if (error = CloseConnection(msg.id);
               error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError Server::HandlePlayerMessages() {
  MaybeError error;
  std::optional<Game::Trick> trick = game.GetCurrentTrick();
  bool isTrickReceived = false;

  if (!trick.has_value()) {
    return Game::NotStarted("Server::HandlePlayerMessages");
  }

  for (ConnectionStore::Message &msg : updateRes.msgs) {
    if (!playerMap.contains(msg.id)) {
      continue;
    }
    try {
      auto m = Message::Deserialize(std::move(msg.content));
      if (m != nullptr) {
        MessageTrick &msgTrick = dynamic_cast<MessageTrick &>(*m);
        Seat seat = playerMap.at(msg.id);
        std::vector<Card> cards = msgTrick.GetCards().Get();
        
        if (seat != trick->turn || msgTrick.GetTrickNumber() != trick->number
            || cards.size() != 1 || !game.IsMoveLegal(seat, cards.at(0))) {
          MessageWrong msgWrong;
          msgWrong.SetTrickNumber(trick->number);
          updateArg.msgs.emplace_back(msg.id, msgWrong.Str());
          if (error = CloseConnection(msg.id); error.has_value()) {
            return error;
          }
        }

        std::optional<Game::TrickResult> result;
        if (MaybeError error = game.Play(cards.at(0), result);
            error.has_value()) {
          return error;
        }

        isTrickReceived = true;
        trick = game.GetCurrentTrick();

        if (!result.has_value()) {
          continue;
        }

        // Finished trick.
        if (error = points.at(result->taker.GetIndex()).Add(result->points);
            error.has_value()) {
          return error;
        }

        if (trick.has_value()) {
          std::array<Card, 4> cardsArr = {trick->cards.at(0),
                                          trick->cards.at(1),
                                          trick->cards.at(2),
                                          cards.at(0)};
          tricks.emplace_back(std::move(cardsArr), result->taker);
        } else {
          MessagePoints msgPoints("SCORE");
          std::array<int, 4> score;
          std::array<int, 4> total;

          for (size_t i = 0; i < 4; i++) {
            score.at(i) = points.at(i).GetScore();
            total.at(i) = points.at(i).GetTotal();
          }
          msgPoints.SetPoints(score);
          for (auto const &c : connections) {
            updateArg.msgs.emplace_back(c.socketFd, msgPoints.Str());
          }
          msgPoints.SetHeader("TOTAL");
          msgPoints.SetPoints(total);
          for (auto const &c : connections) {
            updateArg.msgs.emplace_back(c.socketFd, msgPoints.Str());
          }

          deals.pop_back();
          tricks.clear();

          if (deals.empty()) {
            continue;
          }
          
          if (error = NewDeal(); error.has_value()) {
            return error;
          }
        }
      } else if (error = CloseConnection(msg.id); error.has_value()) {
        return error;
      }
    } catch (std::out_of_range &e) {
      return Error::OutOfRange("Server::HandlePlayerMessages");
    } catch (std::bad_cast &e) {
      if (error = CloseConnection(msg.id); error.has_value()) {
        return error;
      }
    }
  }

  if (!trick.has_value() || !seatMap.at(trick->turn.GetIndex())) {
    return std::nullopt;
  }

  bool isOverdue;
  if (error = IsResponseOverdue(trick->turn, isOverdue); error.has_value()) {
    return error;
  }

  if (!isTrickReceived && isOverdue) {
    if (error = SendTrick(); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError Server::PushCandidate(int socketFd) {
  if (candidateMap.contains(socketFd)) {
    return ErrorSocket("Server::PushCandidate");
  }

  connections.emplace_back(socketFd, Deadline());
  connections.back().responseDeadline->Set(timeout);
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
  nPlayers++;
  return std::nullopt;
}

MaybeError Server::PopConnection(int socketFd) {
  size_t index = 0;

  if (candidateMap.contains(socketFd)) {
    index = candidateMap.at(socketFd);
    if (index != connections.size() - 1) {
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
    if (index != connections.size() - 1) {
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
    seatMap.at(seat.GetIndex()).reset();
    nPlayers--;
  } else {
    return ErrorSocket("Server::PopConnection");
  }

  connections.pop_back();
  return std::nullopt;
}

MaybeError Server::CloseConnection(int socketFd) {
  updateArg.closed.push_back(socketFd);
  return PopConnection(socketFd);
}

MaybeError Server::NewDeal() {
  MessageDeal msgDeal;
  Seat seat;
  int socketFd;

  const auto &hands = deals.back().GetHands();
  msgDeal.SetFirst(deals.back().GetFirst());
  msgDeal.SetType(deals.back().GetType());
  for (int i = 0; i < 4; i++) {
    if (MaybeError error = GetPlayerFd(seat, socketFd); error.has_value()) {
      return error;
    }
    msgDeal.SetHand(hands.at(i));
    updateArg.msgs.emplace_back(socketFd, msgDeal.Str());
    seat.CycleClockwise();
  }
  return game.Deal(deals.back());
}

MaybeError Server::SendTrick() {
  MaybeError error;
  const auto &trick = game.GetCurrentTrick();

  if (!trick.has_value()) {
    return Game::NotStarted("Server::SendTrick");
  }

  Hand hand;
  hand.Set(trick->cards.begin(), trick->cards.end());

  int socketFd;
  if (error = GetPlayerFd(trick->turn, socketFd); error.has_value()) {
    return error;
  }

  MessageTrick msg;
  msg.SetTrickNumber(trick->number);
  msg.SetCards(std::move(hand));
  updateArg.msgs.emplace_back(socketFd, msg.Str());
  return ResetResponseDeadline(trick->turn);
}

MaybeError Server::ResetResponseDeadline(Seat seat) {
  if (!seatMap.at(seat.GetIndex()).has_value()) {
    return ErrorEmptySeat("Server::ResetResponseDeadline");
  }

  Deadline deadline;
  deadline.Set(timeout);

  try {
    connections.at(*seatMap.at(seat.GetIndex())).responseDeadline = deadline;
  } catch (std::out_of_range &e) {
    return Error::OutOfRange("Server::ResetResponseDeadline");
  }

  return std::nullopt;
}

MaybeError Server::IsResponseOverdue(Seat seat, bool &isOverdue) const {
  if (!seatMap.at(seat.GetIndex()).has_value()) {
    return ErrorEmptySeat("Server::IsResponseOverdue");
  }

  try {
    isOverdue = connections.at(*seatMap.at(seat.GetIndex()))
      .responseDeadline->IsOverdue();
  } catch (std::out_of_range &e) {
    return Error::OutOfRange("Server::IsResponseOverdue");
  }

  return std::nullopt;
}

MaybeError Server::GetPlayerFd(Seat seat, int &socketFd) const {
  if (!seatMap.at(seat.GetIndex()).has_value()) {
    return ErrorEmptySeat("Server::GetPlayerFd");
  }

  try {
    socketFd = connections.at(*seatMap.at(seat.GetIndex())).socketFd;
    return std::nullopt;
  } catch (std::out_of_range &e) {
    return Error::OutOfRange("Server::GetPlayerFd");
  }
}

std::unique_ptr<Error> Server::ErrorSocket(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Socket errror.");
}

std::unique_ptr<Error> Server::ErrorEmptySeat(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Seat empty.");
}

void Server::Deadline::Set(std::chrono::milliseconds ms) {
  deadline = std::chrono::system_clock::now() + ms;
}

std::chrono::milliseconds Server::Deadline::GetTimeLeft() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::system_clock::now());
}

bool Server::Deadline::IsOverdue() const {
  return GetTimeLeft().count() <= 0;
}

MaybeError Server::Points::Add(int x) {
  if (x < 0) {
    return std::make_unique<Error>("Server::Points::Add",
                                   "Arg should be non-negative.");
  }

  score += x;
  total += x;
  return std::nullopt;
}

int Server::Points::GetScore() const {
  return score;
}

int Server::Points::GetTotal() const {
  return total;
}

