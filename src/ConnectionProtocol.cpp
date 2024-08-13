//
// Created by robert-grigoryan on 7/11/24.
//

#include "ConnectionProtocol.h"

std::ostream &operator<<(std::ostream &os, const ConnectionProtocol &protocol) {
  switch (protocol) {
    case ConnectionProtocol::kIPv4:
      return os << "kIPv4";
    case ConnectionProtocol::kIPv6:
      return os << "kIPv6";
    default:
      return os << "Unknown protocol";
  }
}
