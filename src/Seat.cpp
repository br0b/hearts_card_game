//
// Created by robert-grigoryan on 5/30/24.
//

#include "Seat.h"

std::unordered_map<char, Seat::Position> Seat::charToPosition = {
    {'N', Position::kN},
    {'E', Position::kE},
    {'S', Position::kS},
    {'W', Position::kW}};

std::unordered_map<Seat::Position, char> Seat::positionToChar = {
    {Position::kN, 'N'},
    {Position::kE, 'E'},
    {Position::kS, 'S'},
    {Position::kW, 'W'}};

std::optional<Seat> Seat::SeatFromChar(char _pos) {
  if (charToPosition.contains(_pos)) {
    return Seat(charToPosition[_pos]);
  }

  return std::nullopt;
}

[[nodiscard]] char Seat::getPositionAsChar() const {
  return positionToChar[position];
}

std::ostream& operator<<(std::ostream& os, const Seat& seat) {
  os << "Seat{position=" << seat.getPositionAsChar() << "}";
  return os;
}
