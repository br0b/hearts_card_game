#include "MessageBusy.h"
#include "MaybeError.h"

void MessageBusy::SetSeats(std::vector<Seat> seats_) {
  seats = std::move(seats_);
}

const std::vector<Seat>& MessageBusy::GetSeats() const {
  return seats;
}

std::ostream& operator<<(std::ostream &os, const MessageBusy &msg) {
  os << "BUSY";

  for (Seat s : msg.seats) {
    os << s;
  }

  return os;
}

MaybeError MessageBusy::SetAfterMatch(std::smatch match) {
  std::regex pattern("[NESW]");  
  std::string str = match[1];
  MaybeError error;

  seats.clear();

  for (auto it = std::sregex_iterator(str.begin(), str.end(), pattern);
       it != std::sregex_iterator();
       it++) {
    seats.emplace_back();
    if (error = seats.back().Parse(it->str()); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

std::string MessageBusy::GetPattern() {
  return "^BUSY([NESW]+)$";
}

