#include "MessageIam.h"

void MessageIam::SetSeat(Seat seat_) {
  seat = seat_;
}

Seat MessageIam::GetSeat() const {
  return seat;
}

std::ostream& operator<<(std::ostream &os, const MessageIam &msg) {
  os << "IAM" << msg.seat;
  return os;
}

MaybeError MessageIam::SetAfterMatch(std::smatch match) {
  return seat.Parse(match[1]);
}

std::string MessageIam::GetPattern() {
  return "^IAM([NESW])$";
}

