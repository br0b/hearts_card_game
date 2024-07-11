//
// Created by robert-grigoryan on 5/30/24.
//

#include "PortConfig.h"

std::optional<uint16_t> PortConfig::getPort() const {
  return port;
}

std::string PortConfig::toString() const{
  return port.has_value() ? std::to_string(port.value()) : "auto";
}

std::ostream &operator<<(std::ostream &os, const PortConfig &port) {
  return os << port.toString();
}
