//
// Created by robert-grigoryan on 6/8/24.
//
#include <iomanip>

#include "Logger.h"
#include "MessageBUSY.h"

std::unique_ptr<Message> MessageBUSY::FromVector(
    const std::vector<Seat>& takenSeats) {
  std::ostringstream oss;
  oss << "BUSY";
  for (Seat seat : takenSeats) {
    oss << seat.serialize();
  }
  oss << "\r\n";
  return std::unique_ptr<MessageBUSY>(new MessageBUSY(oss.str(), takenSeats));
}

std::string MessageBUSY::getMessage() const { return Message::getMessage(); }

MessageBUSY::MessageBUSY(const std::string& _message,
                         const std::vector<Seat>& _takenSeats)
    : Message(_message), takenSeats(_takenSeats) {}
