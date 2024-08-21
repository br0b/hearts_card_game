#include <sstream>

#include "MessageBusy.h"
#include "MaybeError.h"
#include "Utilities.h"

void MessageBusy::SetSeats(std::vector<Seat> seats_) {
  seats = std::move(seats_);
}

const std::vector<Seat>& MessageBusy::GetSeats() const {
  return seats;
}

std::string MessageBusy::Str() const {
  std::ostringstream oss;
  oss << "BUSY";

  for (Seat s : seats) {
    oss << s;
  }

  return oss.str();
}

std::optional<std::string> MessageBusy::UserStr() const {
  std::ostringstream oss;
  oss << "Place busy, list of busy places received: ";
  Utilities::StrList(oss, seats.begin(), seats.end());
  oss << '.';
  return oss.str();
}

MaybeError MessageBusy::SetAfterMatch(std::smatch match) {
  std::regex pattern("[NESW]");  
  std::string str = match[1];

  seats.clear();

  for (auto it = std::sregex_iterator(str.begin(), str.end(), pattern);
       it != std::sregex_iterator();
       it++) {
    seats.emplace_back();
    if (MaybeError error = seats.back().Parse(it->str()); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

std::string MessageBusy::GetPattern() const {
  return "^BUSY([NESW]+)$";
}

