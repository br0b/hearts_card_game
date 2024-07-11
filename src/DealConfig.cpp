//
// Created by robert-grigoryan on 5/27/24.
//

#include <iostream>

#include "DealConfig.h"

DealType DealConfig::getDealType() const{
  return dealType;
}

Seat::Position DealConfig::getFirstPlayer() const {
  return firstPlayer;
}

HandConfig DealConfig::getHandConfig(const Seat::Position player) const {
  return handsConfig.getHandConfig(player);
}

PlayerHandsConfig DealConfig::getHandsConfig() const {
  return handsConfig;
}

std::ostream& operator<< (std::ostream &os, const DealConfig &config) {
  os << "GameConfig{gameType=" << config.dealType
     << ", seat=" << Seat(config.firstPlayer)
     << ", playerHandsConfig=" << config.handsConfig << "}";
  return os;
}