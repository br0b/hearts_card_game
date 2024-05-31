//
// Created by robert-grigoryan on 5/27/24.
//

#include "HandConfig.h"

HandConfig::HandConfig(const std::unordered_set<std::string> &_cards)
    : cards(_cards) {}

std::unordered_set<std::string> HandConfig::getCards() const { return cards; }

std::ostream &operator<<(std::ostream &os, const HandConfig &config) {
  os << "HandConfig{cards=[";
  for (const auto &card : config.getCards()) {
    os << card << ", ";
  }
  return os << "]}";
}
