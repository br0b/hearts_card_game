#include <fcntl.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "Client.h"
#include "ConnectionProtocol.h"
#include "Game.h"
#include "MaybeError.h"
#include "MessageBuffer.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessagePlayTrick.h"
#include "MessageTaken.h"
#include "MessageTrick.h"
#include "TrickNumber.h"
#include "Utilities.h"

Client::Client(Seat seat) : seat(seat) {}

MaybeError Client::Connect(
    std::string host,
    in_port_t port,
    std::optional<ConnectionProtocol> protocol) {
  struct sockaddr_storage serverAddr;
  Utilities::Socket s;
  MaybeError error = std::nullopt;
  int addrFam = GetAddressFamily(protocol);

  if (error = Utilities::CreateAddress(host, port, addrFam, serverAddr);
      error.has_value()) {
    return error;
  }

  if (error = Utilities::GetBoundSocket(addrFam, s); error.has_value()) {
    return error;
  }

  if (error = Utilities::ConnectSocket(s.fd.value(), serverAddr);
      error.has_value()) {
    return error;
  }

  pollfds.front() = {s.fd.value(), POLLIN, 0};

  if (fcntl(pollfds.at(kServerId).fd, F_SETFD, O_NONBLOCK) != 0) {
    return Error::FromErrno("ConnectionStore::UpdateListening");
  }
  
  return server.SetSocket(s.fd.value());
}

MaybeError Client::Run(bool isAutomatic) {
  MaybeError error = std::nullopt;
  MessageIam msgIam;
  msgIam.SetSeat(seat);
  server.PushMessage(msgIam.Str());
  pollfds.at(kServerId).events |= POLLOUT;
  int pollRet = -1;
  std::optional<std::string> msg;

  if (!isAutomatic) {
    pollfds.at(kUserId) = {STDIN_FILENO, POLLIN, 0};
    server.DisableReporting();
    user.DisableReporting();
    user.SetPipe(STDIN_FILENO);
    user.SetSeperator("\n");
  }
  
  while (true) {
    pollRet = poll(pollfds.data(), pollfds.size(), -1);

    if (pollRet < 0) {
      return Error::FromErrno("poll");
    } else if (pollRet == 0) {
      return std::make_unique<Error>("Client::Run",
                                     "Poll returned with no revents");
    }

    if (pollfds.at(kUserId).revents & POLLIN) {
      if (error = user.Receive(); error.has_value()) {
        return error;
      }

      std::optional<std::string> msg = user.PopMessage();
      if (msg.has_value()) {
        if (msg.value() == "cards") {
          PrintCards();
        } else if (msg.value() == "tricks") {
          std::cout << "Taken cards:\n";
          std::ostringstream oss;
          for (size_t i = 0; i < taken.size(); i += 4) {
            std::array<Card, 4> trick;
            std::copy_n(taken.begin() + i, 4, trick.begin());
            Utilities::StrList(oss, trick.begin(), trick.end());
            oss << '\n';
          }
          std::cout << oss.str();
        } else if (msg.value().compare(0, 1, "!") == 0) {
          auto m = Message::Deserialize(msg.value());
          if (m != nullptr) {
            try {
              const MessagePlayTrick &msgPlay
                = dynamic_cast<MessagePlayTrick &>(*m);
              if (!isUserTrickNeeded) {
                std::cout << "The server hasn't sent a trick message yet.\n";
              } else {
                if (error = PlayTrick(msgPlay.GetCard()); error.has_value()) {
                  return error;
                }
                isUserTrickNeeded = false;
              }
            } catch (std::bad_cast &e) {
              PrintHelp();
            }
          } else {
            PrintHelp();
          }
        } else {
          PrintHelp();
        }
      }
    }

    if (pollfds.at(kServerId).revents & POLLIN) {
      if (error = server.Receive(); error.has_value()) {
        return error;
      }

      if (!server.IsOpen()) {
        if (nPointMessagesReceived == 2) {
          close(pollfds.at(kServerId).fd);
          return std::nullopt;
        } else {
          return std::make_unique<Error>("Client::Run",
                                         "Premature disconnection");
        }
      }

      while ((msg = server.PopMessage()).has_value()) {
        if (error = HandleServerMessage(msg.value()); error.has_value()) {
          return error;
        }

        if (pollfds.at(kServerId).fd == -1) {
          return std::nullopt;
        }
      }
    }

    if (pollfds.at(kServerId).revents & POLLOUT) {
      if (error = server.Send(); error.has_value()) {
        return error;
      }
      if (server.IsOutgoingEmpty()) {
        pollfds.at(kServerId).events &= ~POLLOUT;
      }
    }
  }

  return std::nullopt;
}

void Client::PrintCards() const {
  std::vector<Card> tmp;
  std::ostringstream oss;
  oss << "Available: ";
  for (const auto &colorCards : cards) {
    tmp.insert(tmp.end(), colorCards.begin(), colorCards.end());
  }
  Utilities::StrList(oss, tmp.begin(), tmp.end());
  std::cout << oss.str() << '\n';
}

void Client::PrintUserStr(const Message *msg) const {
  if (msg == nullptr) {
    return;
  }

  auto str = msg->UserStr();
  if (str.has_value()) {
    std::cout << str.value() << '\n';
  }
}

void Client::PrintHelp() const {
  std::cout << "!<card> - play a card, for example \"!AS\"\n"
            << "cards – show cards on hand\n"
            << "tricks - show taken cards\n";
}

MaybeError Client::HandleServerMessage(std::string msg) {
  MaybeError error;
  std::unique_ptr<Message> m;
  std::string userStr;
  bool isMsgTrick = false;

  if ((m = Message::Deserialize(msg)) == nullptr) {
    return std::nullopt;
  }

  try {
    if (msg.compare(0, 4, "BUSY") == 0) {
      close(pollfds.front().fd);
      pollfds.at(kServerId).fd = -1;
    } else if (msg.compare(0, 4, "DEAL") == 0) {
      const MessageDeal &msgDeal = dynamic_cast<MessageDeal &>(*m);
      
      for (Card c : msgDeal.GetHand().Get()) {
        cards.at(c.GetColorIndex()).insert(c);
      }

      nPointMessagesReceived = 0;
      nextTrickNumber = TrickNumber();
    } else if (msg.compare(0, 5, "SCORE") == 0
               || msg.compare(0, 5, "TOTAL") == 0) {
      nPointMessagesReceived++;
    } else if (msg.compare(0, 5, "TAKEN") == 0) {
      const MessageTaken &msgTaken = dynamic_cast<MessageTaken &>(*m);
      if (seat == msgTaken.GetTaker()) {
        taken.insert(taken.end(), msgTaken.GetCards().Get().begin(),
                     msgTaken.GetCards().Get().end());
      }
      for (Card card : msgTaken.GetCards().Get()) {
        cards.at(card.GetColorIndex()).erase(card);
      }
      int trickNum = msgTaken.GetTrickNumber().Get();
      if (trickNum == 13) {
        nextTrickNumber.reset();
      } else {
        nextTrickNumber = TrickNumber();
        if (error = nextTrickNumber.value().Set(trickNum + 1);
            error.has_value()) {
          return error;
        }
      }
    } else if (msg.compare(0, 5, "TRICK") == 0) {
      const MessageTrick &msgTrick = dynamic_cast<MessageTrick &>(*m);
      isMsgTrick = true;
      if (nextTrickNumber.has_value()
          && nextTrickNumber.value() == msgTrick.GetTrickNumber()) {
        if (pollfds.at(kUserId).fd == -1) {
          std::optional<Card> choice;
          int colorBeginIndex = 0;
          if (!msgTrick.GetCards().Get().empty()) {
            colorBeginIndex
              = msgTrick.GetCards().Get().front().GetColorIndex();
          }
          for (size_t i = 0; !choice.has_value() && i < 4; i++) {
            size_t index = (colorBeginIndex + i) % 4;
            if (!cards.at(index).empty()) {
              choice = *cards.at(index).begin();
            }
          }
          if (choice.has_value()) {
            if (error = PlayTrick(choice.value()); error.has_value()) {
              return error;
            }
          }
        } else {
          isUserTrickNeeded = true;
          PrintUserStr(m.get());
          PrintCards();
        }
      }
    }
  } catch (std::bad_cast &e) {
    return std::nullopt;
  }

  if (pollfds.at(kUserId).fd != -1 && !isMsgTrick) {
    PrintUserStr(m.get());
  }

  return std::nullopt;
}

MaybeError Client::PlayTrick(Card card) {
  if (!nextTrickNumber.has_value()) {
    return Game::ErrorNotStarted("Client::PlayTrick");
  }

  std::array<Card, 1> tmp = {card};
  Hand hand;
  hand.Set(tmp);
  MessageTrick msgTrick;
  msgTrick.SetTrickNumber(nextTrickNumber.value());
  msgTrick.SetCards(hand);
  server.PushMessage(msgTrick.Str());
  pollfds.at(kServerId).events |= POLLOUT;
  return std::nullopt;
}

int Client::GetAddressFamily(std::optional<ConnectionProtocol> protocol) {
  if (!protocol.has_value()) {
    return AF_UNSPEC;
  } else if (protocol == ConnectionProtocol::kIPv4) {
    return AF_INET;
  } else {
    return AF_INET6;
  }
}

