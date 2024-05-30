//
// Created by robert-grigoryan on 5/27/24.
//
#include "ServerNetworkingConfig.h"

Port ServerNetworkingConfig::getPort() const {
  return port;
}

int ServerNetworkingConfig::getTimeout() const {
  return timeout;
}

std::ostream &operator<<(std::ostream &os,
                         const ServerNetworkingConfig &config) {
  return os << "ServerNetworkingConfig{port=" << config.getPort()
            << ", timeout=" << config.getTimeout() << "}";
}
