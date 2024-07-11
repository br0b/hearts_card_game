//
// Created by robert-grigoryan on 6/7/24.
//
#include <Logger.h>

#include "../include/MessageIAM.h"

std::variant<std::unique_ptr<MessageIAM>, Error> MessageIAM::FromPlayerMessge(
    const std::unique_ptr<Message>& _message) {
  std::string msg = _message->getMessage();
  Error err("ERROR: Invalid message: " +
            _message->getMessageWithoutSeperator());
  if (!std::regex_match(_message->getMessage(), pattern)) {
    return err;
  }

  const std::optional<Seat> seat = Seat::SeatFromChar(msg[3]);
  if (!seat.has_value()) {
    return err;
  }

  return std::unique_ptr<MessageIAM>(
      new MessageIAM(msg, seat.value().getPosition()));
}

Seat::Position MessageIAM::getPosition() const { return position; }

MessageIAM::MessageIAM(const std::string& _message,
                       const Seat::Position _position)
    : Message(_message), position(_position) {}

const std::regex MessageIAM::pattern("^IAM[NESW]\\r\\n$");
