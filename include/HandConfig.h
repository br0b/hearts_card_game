//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef HANDCONFIG_H
#define HANDCONFIG_H

#include <iostream>
#include <unordered_set>

#include "Card.h"

/**
 * Configuration of a player's hand
 */
class HandConfig {
 public:
  [[nodiscard]] std::unordered_set<std::string> getCards() const;

  HandConfig() = default;
  explicit HandConfig(const std::unordered_set<std::string> &_cards);

 private:
  std::unordered_set<std::string> cards;
};

std::ostream &operator<<(std::ostream &os, const HandConfig &config);

#endif  // HANDCONFIG_H
