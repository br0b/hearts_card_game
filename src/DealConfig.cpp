#include "MaybeError.h"

#include "DealConfig.h"

MaybeError DealConfig::SetType(char type_) {
  return type.Set(type_);
}

MaybeError DealConfig::SetSeat(char seat) {
  return first.Set(seat);
}

MaybeError DealConfig::SetHands(const std::vector<std::string> &hands_) {
  MaybeError error = std::nullopt;
  Hand hand;

  hands.clear();
  for (const std::string &hand : hands_) {
    hands.emplace_back();
    if (error = hands.back().Set(hand); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

DealType DealConfig::GetType() const {
  return type;
}

Seat DealConfig::GetFirst() const {
  return first;
}

const Hand& DealConfig::GetHand(int i) const {
  return hands[i];
}

