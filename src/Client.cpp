#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory>

#include "Client.h"
#include "ConnectionProtocol.h"
#include "MaybeError.h"
#include "Message.h"
#include "MessageBuffer.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessageTaken.h"
#include "MessageTrick.h"
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
    user.SetPipe(STDIN_FILENO);
  }
  
  while (true) {
    pollRet = poll(pollfds.data(), pollfds.size(), -1);

    if (pollRet < 0) {
      return Error::FromErrno("poll");
    } else if (pollRet == 0) {
      return std::make_unique<Error>("Client::Run",
                                     "Poll returned with no revents");
    }

    if (pollfds.at(kServerId).revents & POLLIN) {
      if (error = server.Receive(); error.has_value()) {
        return error;
      }
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

    do {
      if ((msg = server.PopMessage()).has_value()) {
        if (error = HandleServerMessage(msg.value()); error.has_value()) {
          return error;
        }

        if (pollfds.at(kServerId).fd == -1) {
          return std::nullopt;
        }
      }
    } while (msg.has_value());

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

MaybeError Client::HandleServerMessage(std::string msg) {
  MaybeError error;
  std::unique_ptr<Message> m;

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
      nextTrickNumber = 1;
    } else if (msg.compare(0, 5, "SCORE") == 0
               || msg.compare(0, 5, "TOTAL") == 0) {
      nPointMessagesReceived++;
    } else if (msg.compare(0, 5, "TAKEN") == 0) {
      const MessageTaken &msgTaken = dynamic_cast<MessageTaken &>(*m);
      if (seat == msgTaken.GetTaker()) {
        taken.insert(taken.end(), msgTaken.GetCards().Get().begin(),
                     msgTaken.GetCards().Get().end());
      }
      nextTrickNumber++;
    } else if (msg.compare(0, 5, "TRICK") == 0) {
      const MessageTrick &msgTrick = dynamic_cast<MessageTrick &>(*m);
      if (nextTrickNumber == msgTrick.GetTrickNumber().Get()) {
        std::optional<Card> choice;
        if (pollfds.at(kUserId).fd == -1) {
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
        } else {
          std::cerr << "User CLI not implemented.\n";
        }

        if (choice.has_value()) {
          cards.at(choice.value().GetColorIndex()).erase(choice.value());
          Hand hand;
          hand.Set(std::array<Card, 1>{choice.value()});
          MessageTrick reply;
          reply.SetTrickNumber(msgTrick.GetTrickNumber());
          reply.SetCards(hand);
          server.PushMessage(reply.Str());
          pollfds.at(kServerId).events |= POLLOUT;
        }
      }
    }
  } catch (std::bad_cast &e) {
    return std::nullopt;
  }

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

