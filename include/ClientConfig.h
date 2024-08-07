//
// Created by robert-grigoryan on 7/11/24.
//
#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <string>

#include "../src/ConnectionProtocol.h"
#include "Seat.h"

class ClientConfig {
public:
  [[nodiscard]] std::string get_host() const;
  [[nodiscard]] int get_port() const;
  [[nodiscard]] ConnectionProtocol getProtocol() const;
  [[nodiscard]] Seat getSeat() const;
  [[nodiscard]] bool getIsAutomatic() const;

  ClientConfig(std::string host, int port, ConnectionProtocol protocol,
               Seat seat, bool isAutomatic);

private:
  std::string host;
  int port;
  ConnectionProtocol protocol;
  Seat seat;
  bool isAutomatic;
};

#endif //CLIENTCONFIG_H
