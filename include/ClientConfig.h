#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <netinet/in.h>
#include <string>
#include <variant>

#include "ConnectionProtocol.h"
#include "Seat.h"

class ClientConfig {
 public:
  ClientConfig(std::string host, in_port_t port, Seat seat);

  [[nodiscard]] static std::variant<std::unique_ptr<ClientConfig>, MaybeError>
    FromMainArgs(int argc, char *argv[]);

  void SetProtocol(ConnectionProtocol protocol_);
  void SetAutomatic();

  [[nodiscard]] std::string GetHost() const;
  [[nodiscard]] in_port_t GetPort() const;
  [[nodiscard]] std::optional<ConnectionProtocol> GetProtocol() const;
  [[nodiscard]] Seat GetSeat() const;
  [[nodiscard]] bool IsAutomatic() const;

 private:
  const std::string host;
  const in_port_t port;
  std::optional<ConnectionProtocol> protocol;
  const Seat seat;
  bool isAutomatic;
};

#endif //CLIENTCONFIG_H
