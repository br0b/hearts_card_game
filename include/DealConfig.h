//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <utility>

#include "DealType.h"
#include "PlayerHandsConfig.h"
#include "Seat.h"

/**
 * Configuration for a single game. One game is composed of at most 13 tricks.
 *
 * @param gameType Type of the game.
 * @param firstPlayer First player in the first round.
 */
class DealConfig {
 public:
  DealConfig(const DealType &_gameType, const Seat _firstPlayer,
             PlayerHandsConfig _handsConfig)
      : dealType(_gameType),
        firstPlayer(_firstPlayer),
        handsConfig(std::move(_handsConfig)) {}

  [[nodiscard]] DealType getGameType() const;
  [[nodiscard]] Seat getFirstPlayer() const;
  [[nodiscard]] PlayerHandsConfig getPlayerHandsConfig() const;

 private:
  DealType dealType;
  Seat firstPlayer;
  PlayerHandsConfig handsConfig;
};

std::ostream &operator<<(std::ostream &os, const DealConfig &config);

#endif  // GAMECONFIG_H
