#include <iostream>
#include <sstream>

#include "MessagePoints.h"
#include "MaybeError.h"
#include "Seat.h"
#include "Utilities.h"

MessagePoints::MessagePoints(std::string header) : header(header) {}

void MessagePoints::SetHeader(std::string header_) {
  header = header_;
}

void MessagePoints::SetPoints(std::array<int, 4> points_) {
  points = std::move(points_);
}

const std::array<int, 4> &MessagePoints::GetPoints() const {
  return points;
}

std::string MessagePoints::Str() const {
  Seat seat;
  std::ostringstream oss;

  oss << header;
  for (int p : points) {
    oss << seat << p;
    seat.CycleClockwise();
  }

  return oss.str();
}

std::optional<std::string> MessagePoints::UserStr() const {
  Seat seat;
  std::ostringstream oss;
  oss << "The "<< ((header == "TOTAL") ? "total " : "") << "scores are:";
  for (int p : points) {
    oss << '\n' << seat << " | " << p;
    seat.CycleClockwise();
  }
  return oss.str();
}

MaybeError MessagePoints::SetAfterMatch(std::smatch match) {
  Seat seat;
  std::optional<int> p;
  std::array<bool, 4> players{false, false, false, false};
  MaybeError error = Error::InvalidArgs("MessagePoints::SetAfterMatch");
  std::string str = match[1].str();
  std::regex pattern("([NESW])(0|[1-9][0-9]*)");

  for (auto it = std::sregex_iterator(str.begin(), str.end(), pattern);
       it != std::sregex_iterator();
       it++) {
    if (MaybeError tmp = seat.Parse((*it)[1]); tmp.has_value()) {
      return tmp;
    }

    // Check if twice the same player.
    if (players.at(seat.GetIndex())) {
      return error;
    } else {
      players.at(seat.GetIndex()) = true;
    }

    if (p = Utilities::ParseNumber((*it)[2], 0, INT_MAX);
        !p.has_value()) {
      return error;
    }

    points.at(seat.GetIndex()) = p.value();
  }

  return std::nullopt;
}

std::string MessagePoints::GetPattern() const { 
  return "^" + header + "((?:[NESW](?:0|[1-9][0-9]*)){4})$";
}

