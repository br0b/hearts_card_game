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
  DealConfig(const DealType &_gameType, const Seat::Position _firstPlayer,
             PlayerHandsConfig _handsConfig)
      : dealType(_gameType),
        firstPlayer(_firstPlayer),
        handsConfig(std::move(_handsConfig)) {}

  [[nodiscard]] DealType getDealType() const;
  [[nodiscard]] Seat::Position getFirstPlayer() const;

  [[nodiscard]] HandConfig getHandConfig(Seat::Position player) const;
  [[nodiscard]] PlayerHandsConfig getHandsConfig() const;

  friend std::ostream& operator<<(std::ostream &os, const DealConfig &config);

 private:
  DealType dealType;
  Seat::Position firstPlayer;
  PlayerHandsConfig handsConfig;
};

#endif  // GAMECONFIG_H
