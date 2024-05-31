//
// Created by robert-grigoryan on 5/27/24.
//
#include "ServerNetworkingConfig.h"

const int ServerNetworkingConfig::kDefaultTimeout = 5;

ServerNetworkingConfig::ServerNetworkingConfig(
    const std::optional<uint16_t> _port, const std::optional<int> _timeout)
    : port(_port), timeout(_timeout.value_or(5)) {}

Port ServerNetworkingConfig::getPort() const { return port; }

int ServerNetworkingConfig::getTimeout() const { return timeout; }

std::string ServerNetworkingConfig::toString() const {
  return "ServerNetworkingConfig{port=" + port.toString() +
         ", timeout=" + std::to_string(timeout) + "}";
}

std::ostream &operator<<(std::ostream &os,
                         const ServerNetworkingConfig &config) {
  return os << "ServerNetworkingConfig{port=" << config.getPort()
            << ", timeout=" << config.getTimeout() << "}";
}
