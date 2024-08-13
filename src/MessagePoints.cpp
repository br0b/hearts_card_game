#include "MessagePoints.h"
#include "MaybeError.h"
#include "Utilities.h"

MessagePoints::MessagePoints(std::string header) : header(header) {}

void MessagePoints::SetPoints(Points points_) {
  points = points_;
}

const Points& MessagePoints::GetPoints() const {
  return points;
}

std::ostream& operator<<(std::ostream &os, const MessagePoints &msg) {
  os << msg.header << msg.points;
  return os;
}

MaybeError MessagePoints::SetAfterMatch(std::smatch match) {
  Seat seat;
  std::optional<int> p;
  std::array<bool, 4> players;
  MaybeError error = Error::InvalidArgs("MessagePoints::SetAfterMatch");
  std::string str = match[1].str();
  std::regex pattern("([NESW])(0|[1-9][0-9]*)");

  points.Clear();

  for (auto it = std::sregex_iterator(str.begin(), str.end(), pattern);
       it != std::sregex_iterator();
       it++) {
    if (MaybeError err = seat.Parse((*it)[1]); error.has_value()) {
      return err;
    }

    // Check if twice the same player.
    if (players[static_cast<size_t>(seat.Get())]) {
      return error;
    } else {
      players[static_cast<size_t>(seat.Get())] = true;
    }

    if (p = Utilities::ParseInt((*it)[2], 0, INT_MAX);
        !p.has_value()) {
      return error;
    }

    points.Add(seat, p.value());
  }

  return std::nullopt;
}

std::string MessagePoints::GetPattern() { 
  return "^" + header + "((?:[NESW](?:0|[1-9][0-9]*)){4})$";
}

