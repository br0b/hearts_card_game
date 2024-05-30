//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef SEAT_H
#define SEAT_H

#include <optional>
#include <ostream>
#include <unordered_map>

class Seat {
 public:
  enum class Position { kN, kE, kS, kW };

  explicit Seat(const Position _position) : position(_position) {}
  static std::optional<Seat> SeatFromChar(char _pos);

  [[nodiscard]] char getPositionAsChar() const;

 private:
  Position position;

  static std::unordered_map<char, Position> charToPosition;
  static std::unordered_map<Position, char> positionToChar;
};

std::ostream& operator<<(std::ostream& os, const Seat& seat);

#endif //SEAT_H
