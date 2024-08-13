#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include "DealType.h"
#include "Hand.h"
#include "Seat.h"

class DealConfig {
 public:
  void SetType(DealType type_);
  void SetFirst(Seat first_);
  void SetHands(std::array<Hand, 4> hands_);

  [[nodiscard]] DealType GetType() const;
  [[nodiscard]] Seat GetFirst() const;
  [[nodiscard]] const std::array<Hand, 4> &GetHands() const;

 private:
  DealType type;
  Seat first;
  std::array<Hand, 4> hands;
};

#endif  // GAMECONFIG_H

