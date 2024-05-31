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
  void playNewDeal(const HandConfig &config);
  std::optional<Error> takeCard(const Card &card);

  [[nodiscard]] int getScore() const;
  [[nodiscard]] int getTotal() const;
  void givePoints(int points);

  PlayerData();

 private:
  std::unordered_set<std::string> hand;
  int score;
  int total;
};

#endif  // PLAYERDATA_H
