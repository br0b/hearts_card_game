//
// Created by robert-grigoryan on 7/11/24.
//
#ifndef CLIENT_H
#define CLIENT_H

#include <optional>

#include "../src/ClientConfig.h"
#include "Error.h"

class Client {
public:
  std::optional<Error> connect();
  std::optional<Error> run();

  explicit Client(ClientConfig config_);

private:
  ClientConfig config;
};

#endif //CLIENT_H
