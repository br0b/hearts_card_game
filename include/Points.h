#ifndef POINTS_H
#define POINTS_H

#include <array>

#include "MaybeError.h"
#include "Seat.h"

class Points {
 public:
  MaybeError Add(Seat player, int x);
  void Clear();

  [[nodiscard]] const std::array<int, 4>& GetPoints() const;

  friend std::ostream& operator<<(std::ostream &os, const Points &points);

 private:
  std::array<int, 4> points;
};

#endif  // POINTS_H
