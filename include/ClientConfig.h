#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <netinet/in.h>
#include <string>

#include "ConnectionProtocol.h"
#include "Seat.h"

class ClientConfig {
 public:
  ClientConfig(std::string host, in_port_t port,
               std::optional<ConnectionProtocol> protocol,
               Seat seat, bool isAutomatic);

  [[nodiscard]] std::string GetHost() const;
  [[nodiscard]] in_port_t GetPort() const;
  [[nodiscard]] std::optional<ConnectionProtocol> GetProtocol() const;
  [[nodiscard]] Seat GetSeat() const;
  [[nodiscard]] bool GetIsAutomatic() const;

 private:
  const std::string host;
  const in_port_t port;
  const std::optional<ConnectionProtocol> protocol;
  const Seat seat;
  const bool isAutomatic;
};

#endif //CLIENTCONFIG_H
