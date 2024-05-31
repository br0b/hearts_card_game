//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <iostream>

#include "DealConfig.h"
#include "ServerNetworkingConfig.h"

class ServerConfig {
public:
  ServerConfig(const std::vector<DealConfig> &_deals,
               const ServerNetworkingConfig &_networkingConfig);

  [[nodiscard]] std::vector<DealConfig> getDeals() const { return deals; }
  [[nodiscard]] ServerNetworkingConfig getNetworkingConfig() const {
    return networkingConfig;
  }

private:
  std::vector<DealConfig> deals;
  ServerNetworkingConfig networkingConfig;
};

std::ostream &operator<<(std::ostream &os, const ServerConfig &config);

#endif  // SERVERCONFIG_H
