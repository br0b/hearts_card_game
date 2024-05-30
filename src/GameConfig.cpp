//
// Created by robert-grigoryan on 5/27/24.
//

#include <iostream>

#include "GameConfig.h"

GameType GameConfig::getGameType() const{
  return gameType;
}

Seat GameConfig::getFirstPlayer() const {
  return firstPlayer;
}

PlayerHandsConfig GameConfig::getPlayerHandsConfig() const {
  return handsConfig;
}

std::ostream& operator<< (std::ostream &os, const GameConfig &config) {
  os << "GameConfig{gameType=" << config.getGameType()
     << ", seat=" << config.getFirstPlayer()
     << ", playerHandsConfig=" << config.getPlayerHandsConfig() << "}";
  return os;
}