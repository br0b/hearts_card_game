//
// Created by robert-grigoryan on 5/27/24.
//
#include <utility>

#include "PlayerHandsConfig.h"

PlayerHandsConfig::PlayerHandsConfig(HandConfig handN, HandConfig handE,
                                     HandConfig handS, HandConfig handW) {
  hands.emplace(Seat::Position::kN, std::move(handN));
  hands.emplace(Seat::Position::kE, std::move(handE));
  hands.emplace(Seat::Position::kS, std::move(handS));
  hands.emplace(Seat::Position::kW, std::move(handW));
}

HandConfig PlayerHandsConfig::getHandConfig(const Seat::Position player) const {
  return hands.at(player);
}

std::ostream &operator<<(std::ostream &os, const PlayerHandsConfig &config) {
  return os << "PlayerHandsConfig{handN="
            << config.getHandConfig(Seat::Position::kN)
            << ", handE=" << config.getHandConfig(Seat::Position::kE)
            << ", handS=" << config.getHandConfig(Seat::Position::kS)
            << ", handW=" << config.getHandConfig(Seat::Position::kW)
            << "}";
}
