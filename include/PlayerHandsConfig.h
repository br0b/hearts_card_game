//
// Created by robert-grigoryan on 5/27/24.
//
#ifndef HANDSCONFIG_H
#define HANDSCONFIG_H

#include <iostream>
#include <utility>

#include "HandConfig.h"

class PlayerHandsConfig {
 public:
  PlayerHandsConfig(HandConfig handN, HandConfig handE, HandConfig handS,
                    HandConfig handW)
      : handN(std::move(handN)),
        handE(std::move(handE)),
        handS(std::move(handS)),
        handW(std::move(handW)) {}

  [[nodiscard]] HandConfig getHandN() const { return handN; }
  [[nodiscard]] HandConfig getHandE() const { return handE; }
  [[nodiscard]] HandConfig getHandS() const { return handS; }
  [[nodiscard]] HandConfig getHandW() const { return handW; }

 private:
  HandConfig handN;
  HandConfig handE;
  HandConfig handS;
  HandConfig handW;
};

std::ostream &operator<<(std::ostream &os, const PlayerHandsConfig &config);

#endif  // HANDSCONFIG_H
