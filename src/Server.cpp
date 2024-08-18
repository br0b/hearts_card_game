#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdexcept>
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
#include "Server.h"

Server::Server() : trickDeadline(maxTimeout) {}

void Server::Configure(std::vector<DealConfig> deals_,
                       std::optional<std::chrono::seconds> maxTimeout_) {
  signal(SIGPIPE, SIG_IGN);
  deals = std::move(deals_);
  // The vector will be used as a stack, so it is initially reversed.
  std::reverse(deals.begin(), deals.end());
  if (maxTimeout_.has_value()) {
    maxTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(
        *maxTimeout_);
  }
}

MaybeError Server::Listen(std::optional<in_port_t> port) {
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
    std::cerr << trickDeadline.GetTimeLeft() << '\n';
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
    if (playerMap.contains(msg.id)) {
      playerMessages.emplace_back(playerMap.at(msg.id), msg.content);
      continue;
    } else if (!candidateMap.contains(msg.id)) {
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
          updateData.msgs.emplace_back(msg.id, msgBusy.Str());
          if (error = CloseConnection(msg.id); error.has_value()) {
            return error;
          }
        } else {
          if (error = PromoteToPlayer(msg.id, msgIam.GetSeat());
                   error.has_value()) {
            return error;
          }
          if (game.GetCurrentTrick().has_value()) {
            Hand hand;
            TrickNumber trickNumber;
            MessageDeal msgDeal;
            MessageTaken msgTaken;
            msgDeal.SetFirst(deals.back().GetFirst());
            msgDeal.SetHand(
                deals.back().GetHands()[msgIam.GetSeat().GetIndex()]);
            msgDeal.SetType(deals.back().GetType());
            updateData.msgs.emplace_back(msg.id, msgDeal.Str());
            for (size_t i = 0; i < tricks.size(); i++) {
              hand.Set(tricks[i].cards.begin(), tricks[i].cards.end());
              msgTaken.SetCards(hand);
              msgTaken.SetTaker(tricks[i].taker);
              if (error = trickNumber.Set(i + 1); error.has_value()) {
                return error;
              }
              updateData.msgs.emplace_back(msg.id, msgTaken.Str());
            }
          }
        }
      } catch (std::bad_cast &e) {
        if (error = CloseConnection(msg.id); error.has_value()) {
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
  int fd;
  std::optional<int> tmpFd;
  std::array<int, 4> playerFds;

  if (!trick.has_value()) {
    return Game::NotStarted("Server::HandlePlayerMessages");
  }

  for (size_t i = 0; i < 4; i++) {
    if (!(tmpFd = players.at(i).GetFd()).has_value()) {
      continue;
    }
    playerFds.at(i) = tmpFd.value();
  }

  const auto msgs = playerMessages;
  playerMessages.clear();
  for (const auto &msg : msgs) {
    if (playerMap.size() != 4) {
      playerMessages.push_back(msg);
    }

    fd = playerFds.at(msg.id.GetIndex());
    try {
      auto m = Message::Deserialize(msg.content);
      if (m != nullptr) {
        MessageTrick &msgTrick = dynamic_cast<MessageTrick &>(*m);
        std::vector<Card> cards = msgTrick.GetCards().Get();
        
        if (msg.id != trick->turn
            || msgTrick.GetTrickNumber() != trick->number
            || cards.size() != 1
            || !game.IsMoveLegal(msg.id, cards.at(0))) {
          MessageWrong msgWrong;
          msgWrong.SetTrickNumber(trick->number);
          updateData.msgs.emplace_back(fd, msgWrong.Str());
          trickDeadline.Reset();
          continue;
        }

        std::optional<Game::TrickResult> result;
        if (MaybeError error = game.Play(cards.at(0), result);
            error.has_value()) {
          return error;
        }

        trick = game.GetCurrentTrick();

        if (!result.has_value()) {
          continue;
        }

        // Finished trick.
        players.at(msg.id.GetIndex()).AddPoints(result->points);

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
            score.at(i) = players.at(i).GetScore();
            total.at(i) = players.at(i).GetTotal();
            players.at(i).ResetScore();
          }
          for (int fd : playerFds) {
            updateData.msgs.emplace_back(fd, msgPoints.Str());
            msgPoints.SetHeader("TOTAL");
            msgPoints.SetPoints(total);
            updateData.msgs.emplace_back(fd, msgPoints.Str());
            msgPoints.SetHeader("SCORE");
            msgPoints.SetPoints(score);
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
        // Broken invariant.
        if (error = CloseConnection(fd); error.has_value()) {
          return error;
        }
        continue;
      }
    } catch (std::out_of_range &e) {
      return Error::OutOfRange("Server::HandlePlayerMessages");
    } catch (std::bad_cast &e) {
      // Broken invariant.
      if (error = CloseConnection(fd); error.has_value()) {
        return error;
      }
      continue;
    }
  }

  if (trick.has_value() && trickDeadline.IsOverdue()) {
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

MaybeError Server::SendTrick() {
  MaybeError error;
  const auto &trick = game.GetCurrentTrick();

  if (!trick.has_value()) {
    return Game::NotStarted("Server::SendTrick");
  }

  Hand hand;
  hand.Set(trick->cards.begin(), trick->cards.end());

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

