//
// Created by robert-grigoryan on 6/7/24.
//
#ifndef MESSAGEIAM_H
#define MESSAGEIAM_H
#include <regex>

#include "Message.h"
#include "Seat.h"

class MessageIAM final : public Message {
public:
  static std::variant<std::unique_ptr<MessageIAM>, Error> FromPlayerMessge(
      const std::unique_ptr<Message>& _message);
  [[nodiscard]] Seat::Position getPosition() const;

private:
  explicit MessageIAM(const std::string& _message,
                      Seat::Position _position);
  Seat::Position position;

  static constexpr size_t kMessageSize = 6;
  static const std::regex pattern;
};

#endif  // MESSAGEIAM_H
