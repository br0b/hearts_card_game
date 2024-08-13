#include "ClientConfig.h"

std::string ClientConfig::GetHost() const {
    return host;
}

in_port_t ClientConfig::GetPort() const {
    return port;
}

std::optional<ConnectionProtocol> ClientConfig::GetProtocol() const {
    return protocol;
}

Seat ClientConfig::GetSeat() const {
    return seat;
}

bool ClientConfig::GetIsAutomatic() const {
    return isAutomatic;
}

ClientConfig::ClientConfig(std::string host, in_port_t port,
                           std::optional<ConnectionProtocol> protocol,
                           Seat seat,
                           bool isAutomatic)
  : host(std::move(host)), port(port), protocol(protocol), seat(seat),
    isAutomatic(isAutomatic) {}

