//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef PLAYERDATA_H
#define PLAYERDATA_H
#include <HandConfig.h>

#include "Error.h"

class PlayerData {
 public:
  [[nodiscard]] bool hasCard(const Card &card) const;
  [[nodiscard]] bool hasColor(Card::Color color) const;
  std::optional<Error> playNewDeal(const HandConfig &config);
  std::optional<Error> takeCard(const Card &card);

  [[nodiscard]] int getScore() const;
  [[nodiscard]] int getTotal() const;
  void givePoints(int points);

  PlayerData();

 private:
  void clearHand();

  std::unordered_map<Card::Color, std::unordered_set<Card>> hand;
  int score;
  int total;
};

#endif  // PLAYERDATA_H
