//
// Created by robert-grigoryan on 5/27/24.
//

#include <vector>

#include "Card.h"
#include "Seat.h"
#include "Server.h"
#include "ServerConfig.h"

PlayerHandsConfig getPlayerHandsConfig()
{
  std::vector<HandConfig> hands;
  std::unordered_set<std::string> hand;

  for (const Card::Color color : Card::getAllColors()) {
    for (const Card::Value value : Card::getAllValues()) {
      hand.insert(Card(value, color).serialize());
    }

    hands.emplace_back(hand);
    hand.clear();
  }

  return {hands[0], hands[1], hands[2], hands[3]};
}

ServerConfig getServerConfig() {
  const DealType dealType(DealType::Type::kTricksBad);
  const Seat firstPlayer(Seat::Position::kN);
  const DealConfig dealConfig(dealType, firstPlayer, getPlayerHandsConfig());
  const std::vector deals = {dealConfig};
  const ServerNetworkingConfig serverNetworkingConfig(std::nullopt, std::nullopt);
  ServerConfig serverConfig(deals, serverNetworkingConfig);
  return serverConfig;
}

int main() {
  Server(getServerConfig(), Logger()).run();
}
