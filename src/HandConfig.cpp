//
// Created by robert-grigoryan on 5/27/24.
//

#include "HandConfig.h"

std::vector<Card> HandConfig::getCards() const{ return cards; }

std::ostream &operator<<(std::ostream &os, const HandConfig &config) {
  os << "HandConfig{cards=[";
  for (const auto &card : config.getCards()) {
    os << card << ", ";
  }
  return os << "]}";
}
