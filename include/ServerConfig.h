//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <iostream>

#include "GameConfig.h"
#include "ServerNetworkingConfig.h"

class ServerConfig {
public:
  ServerConfig(const std::vector<GameConfig> &_games,
               const ServerNetworkingConfig &_networkingConfig)
      : games(_games), networkingConfig(_networkingConfig) {}

  [[nodiscard]] std::vector<GameConfig> getGames() const { return games; }
  [[nodiscard]] ServerNetworkingConfig getNetworkingConfig() const {
    return networkingConfig;
  }

private:
  std::vector<GameConfig> games;
  ServerNetworkingConfig networkingConfig;
};

std::ostream &operator<<(std::ostream &os, const ServerConfig &config);

#endif  // SERVERCONFIG_H
