//
// Created by robert-grigoryan on 5/27/24.
//

#include <iostream>

#include "DealConfig.h"

DealType DealConfig::getGameType() const{
  return dealType;
}

Seat DealConfig::getFirstPlayer() const {
  return firstPlayer;
}

PlayerHandsConfig DealConfig::getPlayerHandsConfig() const {
  return handsConfig;
}

std::ostream& operator<< (std::ostream &os, const DealConfig &config) {
  os << "GameConfig{gameType=" << config.getGameType()
     << ", seat=" << config.getFirstPlayer()
     << ", playerHandsConfig=" << config.getPlayerHandsConfig() << "}";
  return os;
}