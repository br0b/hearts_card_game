//
// Created by robert-grigoryan on 5/27/24.
//
#include "ServerConfig.h"

ServerConfig::ServerConfig(const std::vector<DealConfig> &_deals,
                           const ServerNetworkingConfig &_networkingConfig)
    : deals(_deals), networkingConfig(_networkingConfig) {}

std::ostream &operator<<(std::ostream &os, const ServerConfig &config) {
  os << "ServerConfig{games=[";

  for (const auto &game : config.getDeals()) {
    os << game << ", ";
  }

  return os << "], networkingConfig=" << config.getNetworkingConfig() << "}";
}
