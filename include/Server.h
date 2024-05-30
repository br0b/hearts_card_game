//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef SERVER_H
#define SERVER_H

#include <utility>

#include "ServerConfig.h"

class Server {
 public:
  explicit Server(ServerConfig _config) : config(std::move(_config)) {}
  [[noreturn]] void run();

 private:
  ServerConfig config;
};



#endif //SERVER_H
