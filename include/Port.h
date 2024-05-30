//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef PORT_H
#define PORT_H
#include <cstdint>
#include <optional>
#include <ostream>

class Port {
 public:
  explicit Port(const std::optional<uint16_t> _port) : port(_port) {}
  [[nodiscard]] std::optional<uint16_t> getPort() const;

 private:
  std::optional<uint16_t> port;
};

std::ostream &operator<<(std::ostream &os, const Port &port);

#endif //PORT_H
