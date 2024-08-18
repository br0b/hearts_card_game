#include "ClientConfig.h"
#include "ConnectionProtocol.h"
#include "Utilities.h"

ClientConfig::ClientConfig(std::string host, in_port_t port, Seat seat)
  : host(host), port(port), seat(seat) {}

std::variant<std::unique_ptr<ClientConfig>, MaybeError>
ClientConfig::FromMainArgs(int argc, char *argv[]) {
  std::optional<std::string> host;
  std::optional<in_port_t> port;
  std::optional<ConnectionProtocol> protocol;
  std::optional<Seat> seat;
  bool isAutomatic = false;
  int opt;

  while ((opt = getopt(argc, argv, "h:p:46NESWa")) != -1) {
    switch (opt) {
      case 'h':
        host = optarg;
        break;
      case 'p':
        port = Utilities::ParseNumber<in_port_t>(optarg, 0, UINT16_MAX);
        if (!port.has_value()) {
          return Error::InvalidArg("ClientConfig::FromMainArgs", "port");
        }
        break;
      case '4':
        protocol = ConnectionProtocol::kIPv4;
        break;
      case '6':
        protocol = ConnectionProtocol::kIPv6;
        break;
      case 'N':
      case 'E':
      case 'S':
      case 'W':
        seat = Seat();
        (void)seat->Parse(std::to_string(opt));
        break;
      case 'a':
        isAutomatic = true;
        break;
      default:
        return Error::InvalidArg("ClientConfig::FromMainArgs", optarg);
    }
  }

  if (!host.has_value()) {
    return Error::ArgOmitted("ClientConfig::FromMainArgs", "host");
  } else if (!port.has_value()) {
    return Error::ArgOmitted("ClientConfig::FromMainArgs", "port");
  } else if (!seat.has_value()) {
    return Error::ArgOmitted("ClientConfig::FromMainArgs", "seat");
  }

  auto config = std::make_unique<ClientConfig>(host.value(), port.value(),
                                               seat.value());

  if (protocol.has_value()) {
    config->SetProtocol(protocol.value());
  }

  if (isAutomatic) {
    config->SetAutomatic();
  }

  return config;
}

void ClientConfig::SetProtocol(ConnectionProtocol protocol_) {
  protocol = protocol_;
}

void ClientConfig::SetAutomatic() {
  isAutomatic = true;
}

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

bool ClientConfig::IsAutomatic() const {
    return isAutomatic;
}

