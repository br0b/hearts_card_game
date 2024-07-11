//
// Created by robert-grigoryan on 5/27/24.
//
#ifndef HANDSCONFIG_H
#define HANDSCONFIG_H

#include <iostream>

#include "HandConfig.h"

#include <Seat.h>

class PlayerHandsConfig {
 public:
  PlayerHandsConfig(HandConfig handN, HandConfig handE, HandConfig handS,
                    HandConfig handW);

  [[nodiscard]] HandConfig getHandConfig(Seat::Position player) const;

 private:
  std::unordered_map<Seat::Position, HandConfig> hands;
};

std::ostream &operator<<(std::ostream &os, const PlayerHandsConfig &config);

#endif  // HANDSCONFIG_H
