//
// Created by robert-grigoryan on 5/27/24.
//
#ifndef SERVERNETWORKINGCONFIG_H
#define SERVERNETWORKINGCONFIG_H
#include <cstdint>
#include <optional>

#include "Port.h"

class ServerNetworkingConfig {
 public:
  explicit ServerNetworkingConfig(std::optional<uint16_t> _port,
                                  std::optional<int> _timeout);
  [[nodiscard]] Port getPort() const;
  [[nodiscard]] int getTimeout() const;

  [[nodiscard]] std::string toString() const;

 private:
  static const int kDefaultTimeout;
  Port port;
  int timeout;
};

std::ostream &operator<<(std::ostream &os,
                         const ServerNetworkingConfig &config);

#endif  // SERVERNETWORKINGCONFIG_H
