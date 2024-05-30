//
// Created by robert-grigoryan on 5/27/24.
//

#include <iostream>
#include <vector>

#include "Card.h"
#include "Seat.h"
#include "ServerConfig.h"

PlayerHandsConfig getPlayerHandsConfig()
{
  std::vector<HandConfig> hands;
  std::vector<Card> hand;

  for (Card::Color color : Card::getAllColors()) {
    for (Card::Value value : Card::getAllValues()) {
      hand.emplace_back(value, color);
    }

    hands.emplace_back(hand);
    hand.clear();
  }

  return {hands[0], hands[1], hands[2], hands[3]};
}

ServerConfig getServerConfig() {
  const GameType gameType(GameType::Type::kTricksBad);
  const Seat firstPlayer(Seat::Position::kN);
  const GameConfig gameConfig(gameType, firstPlayer, getPlayerHandsConfig());
  const std::vector games = {gameConfig};
  const ServerNetworkingConfig serverNetworkingConfig(std::nullopt, std::nullopt);
  ServerConfig serverConfig(games, serverNetworkingConfig);
  return serverConfig;
}

int main() {
  new Server(getServerConfig()).run();
}
