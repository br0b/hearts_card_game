//
// Created by robert-grigoryan on 5/27/24.
//
#include <utility>

#include "PlayerHandsConfig.h"

PlayerHandsConfig::PlayerHandsConfig(HandConfig handN, HandConfig handE,
                                     HandConfig handS, HandConfig handW) {
  hands[Seat(Seat::Position::kN).serialize()] = std::move(handN);
  hands[Seat(Seat::Position::kE).serialize()] = std::move(handE);
  hands[Seat(Seat::Position::kS).serialize()] = std::move(handS);
  hands[Seat(Seat::Position::kW).serialize()] = std::move(handW);
}

HandConfig PlayerHandsConfig::getHandConfig(Seat player) const {
  return hands.at(player.serialize());
}

std::ostream &operator<<(std::ostream &os, const PlayerHandsConfig &config) {
  return os << "PlayerHandsConfig{handN="
            << config.getHandConfig(Seat(Seat::Position::kN))
            << ", handE=" << config.getHandConfig(Seat(Seat::Position::kE))
            << ", handS=" << config.getHandConfig(Seat(Seat::Position::kS))
            << ", handW=" << config.getHandConfig(Seat(Seat::Position::kW))
            << "}";
}
