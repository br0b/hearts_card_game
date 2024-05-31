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

Seat::Position Seat::getPosition() const{
  return position;
}

[[nodiscard]] char Seat::serialize() const { return positionToChar[position]; }

std::vector<Seat> Seat::getAllSeats() {
  return {Seat(Position::kN), Seat(Position::kE),
          Seat(Position::kS), Seat(Position::kW)};
}

Seat::Seat(const Position _position) : position(_position) {}

std::ostream& operator<<(std::ostream& os, const Seat& seat) {
  os << "Seat{position=" << seat.serialize() << "}";
  return os;
}
