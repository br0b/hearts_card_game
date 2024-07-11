//
// Created by robert-grigoryan on 5/27/24.
//
#include <Logger.h>

#include <vector>

#include "Card.h"
#include "Seat.h"
#include "Server.h"
#include "ServerConfig.h"

PlayerHandsConfig getPlayerHandsConfig() {
  std::vector<HandConfig> hands;
  std::vector<Card> hand;

  for (const Card::Color color : Card::getAllColors()) {
    for (const Card::Value value : Card::getAllValues()) {
      hand.emplace_back(value, color);
    }

    hands.emplace_back(hand);
    hand.clear();
  }

  return {hands[0], hands[1], hands[2], hands[3]};
}

ServerConfig getServerConfig() {
  const DealType dealType(DealType::Type::kTricksBad);
  constexpr auto firstPlayer = Seat::Position::kN;
  const DealConfig dealConfig(dealType, firstPlayer, getPlayerHandsConfig());
  const std::vector deals = {dealConfig};
  const ServerNetworkingConfig serverNetworkingConfig(42000, 1000000);
  ServerConfig serverConfig(deals, serverNetworkingConfig);
  return serverConfig;
}

int main() {
  std::variant<std::unique_ptr<Server>, Error> server =
      Server::Create(getServerConfig());
  if (std::holds_alternative<Error>(server)) {
    Logger::log(std::get<Error>(server).getMessage());
    return 1;
  }

  if (const std::optional<Error> ret =
          std::get<std::unique_ptr<Server>>(server)->run();
      ret.has_value()) {
    Logger::log(ret.value().getMessage());
    return 1;
  }

  return 0;
}
