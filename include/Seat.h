//
// Created by robert-grigoryan on 5/27/24.
//
#ifndef SEAT_H
#define SEAT_H

#include <optional>
#include <ostream>
#include <unordered_map>
#include <vector>

class Seat {
 public:
  enum class Position { kN, kE, kS, kW };

  [[nodiscard]] Position getPosition() const;
  [[nodiscard]] std::string serialize() const;
  [[nodiscard]] int getRank() const;

  static std::optional<Seat> SeatFromChar(char _pos);
  static std::vector<Seat> getAllSeats();
  explicit Seat(Position _position);

 private:
  Position position;

  static std::unordered_map<char, Position> charToPosition;
  static std::unordered_map<Position, char> positionToChar;
  // Map Positions to numbers. Used for pollfds indexing.
  static std::unordered_map<Position, int> positionToRank;
};

std::ostream& operator<<(std::ostream& os, const Seat& seat);

#endif  // SEAT_H
