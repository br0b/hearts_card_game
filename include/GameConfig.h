//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <utility>

#include "GameType.h"
#include "PlayerHandsConfig.h"
#include "Seat.h"

/**
 * Configuration for a single game. One game is composed of at most 13 tricks.
 *
 * @param gameType Type of the game.
 * @param firstPlayer First player in the first round.
 */
class GameConfig {
 public:
  GameConfig(const GameType &_gameType, const Seat _firstPlayer,
             PlayerHandsConfig _handsConfig)
      : gameType(_gameType),
        firstPlayer(_firstPlayer),
        handsConfig(std::move(_handsConfig)) {}

  [[nodiscard]] GameType getGameType() const;
  [[nodiscard]] Seat getFirstPlayer() const;
  [[nodiscard]] PlayerHandsConfig getPlayerHandsConfig() const;

 private:
  GameType gameType;
  Seat firstPlayer;
  PlayerHandsConfig handsConfig;
};

std::ostream &operator<<(std::ostream &os, const GameConfig &config);

#endif  // GAMECONFIG_H
