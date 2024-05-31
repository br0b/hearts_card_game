//
// Created by robert-grigoryan on 5/30/24.
//

#include "Port.h"

std::optional<uint16_t> Port::getPort() const {
  return port;
}

std::string Port::toString() const{
  return port.has_value() ? std::to_string(port.value()) : "auto";
}

std::ostream &operator<<(std::ostream &os, const Port &port) {
  return os << port.toString();
}
