#ifndef SEAT_H
#define SEAT_H

#include <array>
#include <ostream>

#include "MaybeError.h"

class Seat {
 public:
  enum class Value {
    kN = 0,
    kE,
    kS,
    kW,
  };

  MaybeError Parse(std::string str);

  void Set(Value value_);
  void CycleClockwise();

  [[nodiscard]] Value Get() const;
  friend std::ostream& operator<<(std::ostream &os, const Seat &s);

 private:
  Value value;
  static constexpr std::array<char, 4> chars{'N', 'E', 'S', 'W'};
};

#endif  // SEAT_H

