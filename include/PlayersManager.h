//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef PLAYERHANDSMANAGER_H
#define PLAYERHANDSMANAGER_H
#include <Card.h>
#include <PlayerData.h>
#include <PlayerHandsConfig.h>
#include <Seat.h>

#include <unordered_map>

#include "Error.h"

class PlayersManager {
 public:
  void playNewDeal(const PlayerHandsConfig& config);

  [[nodiscard]] bool hasCard(Seat seat, const Card& card) const;
  std::optional<Error> takeCard(Seat seat, const Card& card);

  [[nodiscard]] int getScore(Seat seat) const;
  [[nodiscard]] int getTotal(Seat seat) const;
  [[nodiscard]] std::optional<Error> distributePoints(
      const std::unordered_map<char, int>& points);

  PlayersManager();

 private:
  std::unordered_map<char, PlayerData> players;
};

#endif  // PLAYERHANDSMANAGER_H