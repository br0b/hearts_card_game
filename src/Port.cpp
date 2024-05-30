//
// Created by robert-grigoryan on 5/30/24.
//

#include "Port.h"

std::optional<uint16_t> Port::getPort() const {
  return port;
}

std::ostream &operator<<(std::ostream &os, const Port &port) {
  if (port.getPort().has_value()) {
    return os << port.getPort().value();
  }

  return os << "auto";
}
