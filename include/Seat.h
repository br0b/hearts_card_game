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
  [[nodiscard]] size_t GetIndex() const;
  friend std::ostream& operator<<(std::ostream &os, const Seat &s);
  friend bool operator==(const Seat &l, const Seat &r);

 private:
  Value value = Value::kN;

  static constexpr std::array<char, 4> chars{'N', 'E', 'S', 'W'};
};

#endif  // SEAT_H

