//
// Created by robert-grigoryan on 5/27/24.
//

#include "ServerConfig.h"

std::ostream &operator<<(std::ostream &os, const ServerConfig &config) {
  os << "ServerConfig{games=[";

  for (const auto &game : config.getGames()) {
    os << game << ", ";
  }

  return (os << "], networkingConfig=" << config.getNetworkingConfig() << "}");
}
