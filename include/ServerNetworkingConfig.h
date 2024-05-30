//
// Created by robert-grigoryan on 5/27/24.
//
#ifndef SERVERNETWORKINGCONFIG_H
#define SERVERNETWORKINGCONFIG_H
#include <cstdint>
#include <iostream>
#include <optional>

#include "Port.h"

class ServerNetworkingConfig {
 public:
  explicit ServerNetworkingConfig(const std::optional<uint16_t> _port,
                                  const std::optional<int> _timeout)
      : port(_port), timeout(_timeout.value_or(5)) {}
  [[nodiscard]] Port getPort() const;
  [[nodiscard]] int getTimeout() const;

 private:
  static constexpr int kDefaultTimeout = 5;
  Port port;
  int timeout;
};

std::ostream &operator<<(std::ostream &os,
                         const ServerNetworkingConfig &config);

#endif  // SERVERNETWORKINGCONFIG_H
