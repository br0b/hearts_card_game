#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>

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
#include "TrickNumber.h"
#include "Server.h"

Server::Server() : trickDeadline(maxTimeout) {}

void Server::Configure(std::vector<DealConfig> deals_,
                       std::optional<std::chrono::seconds> maxTimeout_) {
  deals = std::move(deals_);
  // The vector will be used as a stack, so it is initially reversed.
  std::reverse(deals.begin(), deals.end());
  if (maxTimeout_.has_value()) {
    maxTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(
        *maxTimeout_);
  }
}

MaybeError Server::Listen(std::optional<in_port_t> port) {
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    return Error::FromErrno("signal");
  }

  return connectionStore.Listen(port);
}

MaybeError Server::Run() {
  if (deals.empty()) {
    return std::nullopt;
  }

  MaybeError error;

  // Initate the game.

  // Gather the four players.
  if (error = SafeUpdate(); error.has_value()) {
    return error;
  }

  if (error = NewDeal(); error.has_value()) {
    return error;
  }

  if (error = SendTrick(); error.has_value()) {
    return error;
  }

  while (!deals.empty()) {
    if (error = HandlePlayerMessages(); error.has_value()) {
      return error;
    }

    if (error = SafeUpdate(); error.has_value()) {
      return error; 
    }
  }

  return connectionStore.Close();
}

void Server::EnableDebug() {
  connectionStore.EnableDebug();
  debugMode = true;
}

MaybeError Server::SafeUpdate() {
  // Gather four players.
  do {
    if (MaybeError error = Update(); error.has_value()) {
      return error;
    }
  } while (playerMap.size() < 4);

  return std::nullopt;
}

MaybeError Server::Update() {
  MaybeError error;

  // Set timeout
  std::optional<std::chrono::milliseconds> timeout;

  if (playerMap.size() == 4) {
    timeout = trickDeadline.GetTimeLeft();
  }

  std::vector<int> overdueCandidates;
  for (const Candidate &c: candidates) {
    if (c.deadline.IsOverdue()) {
      overdueCandidates.push_back(c.fd);
    } else {
      auto timeLeft = c.deadline.GetTimeLeft();
      if (!timeout.has_value() || timeLeft < *timeout) {
        timeout = timeLeft;
      }
    }
  }

  // Close overdue candidates.
  for (int fd : overdueCandidates) {
    if (error = CloseConnection(fd); error.has_value()) {
      return error;
    }
  }

  if (error = connectionStore.Update(updateData, timeout); error.has_value()) {
    return error;
  }

  // Pop closed.
  for (int fd : updateData.closed) {
    if (error = PopConnection(fd); error.has_value()) {
      return error;
    }
  }
  updateData.closed.clear();

  // New candidate
  if (updateData.opened.has_value()) {
    if (error = PushCandidate(updateData.opened.value()); error.has_value()) {
      return error;
    }
  }
  updateData.opened.reset();

  const auto msgs = updateData.msgs;
  updateData.msgs.clear();
  for (const auto &msg : msgs) {
    if (playerMap.contains(msg.fd)) {
      playerMessages.emplace_back(msg.fd, msg.content);
      continue;
    } else if (!candidateMap.contains(msg.fd)) {
      return ErrorSocket("Server::Update");
    }

    auto m = Message::Deserialize(std::move(msg.content));
    if (m != nullptr) {
      try {
        MessageIam &msgIam = dynamic_cast<MessageIam &>(*m);
        if (players.at(msgIam.GetSeat().GetIndex()).GetFd().has_value()) {
          std::vector<Seat> vecBusy;
          Seat seat;
          for (const Player &p : players) {
            if (p.GetFd().has_value()) {
              vecBusy.push_back(seat);
            }
            seat.CycleClockwise();
          }
          MessageBusy msgBusy;
          msgBusy.SetSeats(vecBusy);
          updateData.msgs.emplace_back(msg.fd, msgBusy.Str());
          if (error = CloseConnection(msg.fd); error.has_value()) {
            return error;
          }
        } else {
          if (error = PromoteToPlayer(msg.fd, msgIam.GetSeat());
                   error.has_value()) {
            return error;
          }
          if (game.GetCurrentTrick().has_value()) {
            TrickNumber trickNumber;
            MessageDeal msgDeal;
            msgDeal.SetFirst(deals.back().GetFirst());
            msgDeal.SetHand(
                deals.back().GetHands()[msgIam.GetSeat().GetIndex()]);
            msgDeal.SetType(deals.back().GetType());
            updateData.msgs.emplace_back(msg.fd, msgDeal.Str());

            for (size_t i = 1; i <= tricks.size(); i++) {
              if (error = SendTaken(trickNumber, msgIam.GetSeat());
                  error.has_value()) {
                return error;
              }
              if (error = trickNumber.Set(i + 1); error.has_value()) {
                return error;
              }
            }
          }
        }
      } catch (std::bad_cast &e) {
        if (error = CloseConnection(msg.fd); error.has_value()) {
          return error;
        }
      }
    } else if (error = CloseConnection(msg.fd);
               error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError Server::HandlePlayerMessages() {
  MaybeError error;
  const std::optional<Game::Trick> &trick = game.GetCurrentTrick();
  std::optional<int> tmpFd;
  std::array<int, 4> playerFds;
  bool isTrickReceived = false;

  if (!trick.has_value()) {
    return Game::ErrorNotStarted("Server::HandlePlayerMessages");
  }

  for (size_t i = 0; i < 4; i++) {
    if (!(tmpFd = players.at(i).GetFd()).has_value()
        || !playerMap.contains(tmpFd.value())) {
      return ErrorSocket("Server::HandlePlayerMessages");
    }
    playerFds.at(i) = tmpFd.value();
  }

  const auto msgs = playerMessages;
  playerMessages.clear();
  for (const auto &msg : msgs) {
    // If the game has been halted, then store the message for when the game is resumed.
    if (playerMap.size() != 4) {
      if (playerMap.contains(msg.fd)) {
        playerMessages.push_back(msg);
      }
      continue;
    }

    Seat seat = playerMap.at(msg.fd);
    auto m = Message::Deserialize(msg.content);

    try {
      if (m != nullptr) {
        const MessageTrick &msgTrick = dynamic_cast<MessageTrick &>(*m);
        isTrickReceived = true;
        std::vector<Card> cards = msgTrick.GetCards().Get();
        
        if (seat != trick->turn
            || msgTrick.GetTrickNumber() != trick->number
            || cards.size() != 1
            || !game.IsMoveLegal(seat, cards.at(0))) {
          MessageWrong msgWrong;
          msgWrong.SetTrickNumber(trick->number);
          updateData.msgs.emplace_back(msg.fd, msgWrong.Str());
          trickDeadline.Reset();
          continue;
        }

        std::optional<Game::TrickResult> result;
        if (MaybeError error = game.Play(cards.at(0), result);
            error.has_value()) {
          return error;
        }

        if (!result.has_value()) {
          continue;
        }

        // Finished trick.
        tricks.emplace_back(result->cards, result->taker);
        players.at(result->taker.GetIndex()).AddPoints(result->points);
        Seat seat;
        for (int i = 0; i < 4; i++) {
          if (MaybeError error = SendTaken(msgTrick.GetTrickNumber(), seat);
              error.has_value()) {
            return error;
          }
          seat.CycleClockwise();
        }

        // Finished deal.
        if (!trick.has_value()) {
          MessagePoints msgPoints("SCORE");
          std::array<int, 4> score;
          std::array<int, 4> total;

          for (size_t i = 0; i < 4; i++) {
            score.at(i) = players.at(i).GetScore();
            total.at(i) = players.at(i).GetTotal();
            players.at(i).ResetScore();
          }
          for (int fd : playerFds) {
            msgPoints.SetHeader("SCORE");
            msgPoints.SetPoints(score);
            updateData.msgs.emplace_back(fd, msgPoints.Str());
            msgPoints.SetHeader("TOTAL");
            msgPoints.SetPoints(total);
            updateData.msgs.emplace_back(fd, msgPoints.Str());
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
      } else {
        if (error = CloseConnection(msg.fd); error.has_value()) {
          return error;
        }
      }
    } catch (std::bad_cast &e) {
      if (error = CloseConnection(msg.fd); error.has_value()) {
        return error;
      }
    }
  }

  if (playerMap.size() == 4
      && trick.has_value()
      && (isTrickReceived || trickDeadline.IsOverdue())) {
    return SendTrick();
  }

  return std::nullopt;
}

MaybeError Server::PushCandidate(int socketFd) {
  if (candidateMap.contains(socketFd)) {
    return ErrorSocket("Server::PushCandidate");
  }

  candidates.emplace_back(socketFd, Deadline(maxTimeout));
  candidateMap.emplace(socketFd, candidates.size() - 1);
  return std::nullopt;
}

MaybeError Server::PromoteToPlayer(int socketFd, Seat seat) {
  if (!candidateMap.contains(socketFd) || playerMap.contains(socketFd)) {
    return ErrorSocket("Server::PromoteToPlayer");
  }
  
  if (players.at(seat.GetIndex()).GetFd().has_value()) {
    return std::make_unique<Error>("Server::PromoteToPlayer",
                                   "Seat already taken.");
  }

  if (MaybeError error = PopConnection(socketFd); error.has_value()) {
    return error;
  }
  
  players.at(seat.GetIndex()).SetFd(socketFd);
  playerMap.emplace(socketFd, seat);
  return std::nullopt;
}

MaybeError Server::PopConnection(int socketFd) {
  size_t index = 0;

  if (candidateMap.contains(socketFd)) {
    index = candidateMap.at(socketFd);
    if (index != candidates.size() - 1) {
      candidates.at(index).deadline.Set(candidates.back().deadline.Get());
      candidates.at(index).fd = candidates.back().fd;
      candidateMap.at(candidates.back().fd) = index;
    }
    candidates.pop_back();
    candidateMap.erase(socketFd);
  } else if (playerMap.contains(socketFd)) {
    auto tmp = playerMessages;
    Seat seat = playerMap.at(socketFd);
    if (!players.at(seat.GetIndex()).GetFd().has_value()) {
      return ErrorEmptySeat("Server::PopConnection");
    }
    playerMap.erase(socketFd);
    players.at(seat.GetIndex()).SetFd(std::nullopt);
  } else {
    return ErrorSocket("Server::PopConnection");
  }

  return std::nullopt;
}

MaybeError Server::CloseConnection(int socketFd) {
  updateData.closed.push_back(socketFd);
  return PopConnection(socketFd);
}

MaybeError Server::NewDeal() {
  MessageDeal msgDeal;
  std::optional<int> fd;

  const auto &hands = deals.back().GetHands();
  msgDeal.SetFirst(deals.back().GetFirst());
  msgDeal.SetType(deals.back().GetType());
  for (size_t i = 0; i < 4; i++) {
    if (!(fd = players.at(i).GetFd()).has_value()) {
      return ErrorEmptySeat("Server::NewDeal");
    }
    msgDeal.SetHand(hands.at(i));
    updateData.msgs.emplace_back(fd.value(), msgDeal.Str());
  }
  return game.Deal(deals.back());
}

MaybeError Server::SendTaken(TrickNumber trickNumber, Seat seat) {
  size_t trickIndex = trickNumber.Get() - 1;
  std::optional<int> fd;

  if (trickIndex >= tricks.size()) {
    return Error::InvalidArg("Server::SendTaken", "trickNumber");
  } else if (!(fd = players.at(seat.GetIndex()).GetFd()).has_value()) {
    return ErrorEmptySeat("Server::SendTaken");
  }

  auto cards = tricks.at(trickIndex).cards;
  Hand hand;
  hand.Set(cards);

  MessageTaken msgTaken;
  msgTaken.SetTrickNumber(trickNumber);
  msgTaken.SetCards(hand);
  msgTaken.SetTaker(tricks.at(trickIndex).taker);
  updateData.msgs.emplace_back(fd.value(), msgTaken.Str());
  return std::nullopt;
}

MaybeError Server::SendTrick() {
  MaybeError error;
  const auto &trick = game.GetCurrentTrick();

  if (!trick.has_value()) {
    return Game::ErrorNotStarted("Server::SendTrick");
  }

  Hand hand;
  hand.Set(trick->cards);

  std::optional<int> fd;
  if (!(fd = players.at(trick.value().turn.GetIndex()).GetFd()).has_value()) {
    return error;
  }

  MessageTrick msg;
  msg.SetTrickNumber(trick->number);
  msg.SetCards(std::move(hand));
  updateData.msgs.emplace_back(fd.value(), msg.Str());
  trickDeadline.Reset();
  return std::nullopt;
}

std::unique_ptr<Error> Server::ErrorSocket(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Socket error.");
}

std::unique_ptr<Error> Server::ErrorEmptySeat(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Seat empty.");
}

Server::Deadline::Deadline(std::chrono::milliseconds timeout)
    : deadline(std::chrono::system_clock::now() + timeout),
      timeout(timeout) {}

void Server::Deadline::Reset() {
  deadline = std::chrono::system_clock::now() + timeout;
}

void Server::Deadline::Set(TimePoint timePoint) {
  deadline = timePoint;
}

Server::TimePoint Server::Deadline::Get() const {
  return deadline;
}

std::chrono::milliseconds Server::Deadline::GetTimeLeft() const {
  auto ret = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::system_clock::now());
  if (ret < std::chrono::milliseconds(0)) {
    return std::chrono::milliseconds(0);
  }
  return ret;
}

bool Server::Deadline::IsOverdue() const {
  return GetTimeLeft().count() <= 0;
}

Server::Player::Player(Seat seat) : seat{seat} {}

void Server::Player::AddPoints(int x) {
  score += x;
  total += x;
}

void Server::Player::ResetScore() {
  score = 0;
}

void Server::Player::SetFd(std::optional<int> fd_) {
  fd = fd_;
}

int Server::Player::GetScore() const {
  return score;
}

int Server::Player::GetTotal() const {
  return total;
}

std::optional<int> Server::Player::GetFd() const {
  return fd;
}

Seat Server::Player::GetSeat() const {
  return seat;
}

