//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef PORT_H
#define PORT_H
#include <cstdint>
#include <optional>
#include <ostream>

// To be used with ServerConfig.
class PortConfig {
 public:
  explicit PortConfig(const std::optional<uint16_t> _port) : port(_port) {}
  [[nodiscard]] std::optional<uint16_t> getPort() const;

  [[nodiscard]] std::string toString() const;

 private:
  std::optional<uint16_t> port;
};

std::ostream &operator<<(std::ostream &os, const PortConfig &port);

#endif //PORT_H
