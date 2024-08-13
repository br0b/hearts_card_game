#include "Points.h"
#include "MaybeError.h"

MaybeError Points::Add(Seat player, int x) {
  if (x < 1) {
    return Error::InvalidArgs("Points::Add");
  }

  points[static_cast<int>(player.Get())] += x;
  return std::nullopt;
}

void Points::Clear() {
  std::fill(points.begin(), points.end(), 0);
}

std::ostream& operator<<(std::ostream &os, const Points &points) {
  Seat seat;
  seat.Set(Seat::Value::kN);

  for (int p : points.points) {
    os << seat << p;
    seat.CycleClockwise();
  }

  return os;
}

