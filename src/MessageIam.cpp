#include <sstream>

#include "MessageIam.h"

void MessageIam::SetSeat(Seat seat_) {
  seat = seat_;
}

Seat MessageIam::GetSeat() const {
  return seat;
}

std::string MessageIam::Str() const {
  std::ostringstream oss;
  oss << "IAM" << seat;
  return oss.str();
}

std::optional<std::string> MessageIam::UserStr() const {
  return std::nullopt;
}

MaybeError MessageIam::SetAfterMatch(std::smatch match) {
  return seat.Parse(match[1]);
}

std::string MessageIam::GetPattern() const {
  return "^IAM([NESW])$";
}

