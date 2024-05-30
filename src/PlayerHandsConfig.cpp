//
// Created by robert-grigoryan on 5/27/24.
//

#include "PlayerHandsConfig.h"

std::ostream &operator<<(std::ostream &os,
                                const PlayerHandsConfig &config) {
  return os << "PlayerHandsConfig{handN=" << config.getHandN()
            << ", handE=" << config.getHandE()
            << ", handS=" << config.getHandS()
            << ", handW=" << config.getHandW() << "}";
}
