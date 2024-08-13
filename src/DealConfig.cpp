#include "DealConfig.h"

void DealConfig::SetType(DealType type_) {
  type = type_;
}

void DealConfig::SetFirst(Seat first_) {
  first = first_;
}

void DealConfig::SetHands(std::array<Hand, 4> hands_) {
  hands = std::move(hands_);
}

DealType DealConfig::GetType() const {
  return type;
}

Seat DealConfig::GetFirst() const {
  return first;
}

const std::array<Hand, 4> &DealConfig::GetHands() const {
  return hands;
}

