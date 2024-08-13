//
// Created by robert-grigoryan on 5/27/24.
//
#include <iomanip>

#include "HandConfig.h"

HandConfig::HandConfig(const std::vector<Card> &_cards)
    : cards(_cards) {}

std::vector<Card> HandConfig::getCards() const { return cards; }

std::string HandConfig::serialize() const {
  std::ostringstream str;
  for (Card card : cards) {
    str << card.serialize();
  }
  return str.str();
}

std::ostream &operator<<(std::ostream &os, const HandConfig &config) {
  os << "HandConfig{cards=[";
  for (const auto &card : config.getCards()) {
    os << card << ", ";
  }
  return os << "]}";
}
