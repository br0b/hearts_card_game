#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include "DealType.h"
#include "Hand.h"
#include "MaybeError.h"
#include "Seat.h"

class DealConfig {
 public:
  [[nodiscard]] MaybeError SetType(char type_);
  [[nodiscard]] MaybeError SetSeat(char seat);
  [[nodiscard]] MaybeError SetHands(const std::vector<std::string> &hands_);

  [[nodiscard]] DealType GetType() const;
  [[nodiscard]] Seat GetFirst() const;
  [[nodiscard]] const Hand& GetHand(int i) const;

 private:
  DealType type;
  Seat first;
  std::vector<Hand> hands;
};

#endif  // GAMECONFIG_H

