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
  [[nodiscard]] std::vector<Card> getCards() const;
  [[nodiscard]] std::string serialize() const;

  explicit HandConfig(const std::vector<Card> &_cards);

 private:
  std::vector<Card> cards;
};

std::ostream &operator<<(std::ostream &os, const HandConfig &config);

#endif  // HANDCONFIG_H
