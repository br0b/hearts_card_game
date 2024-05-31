//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef SERVER_H
#define SERVER_H

#include <memory>

#include "Game.h"
#include "Logger.h"
#include "ServerConfig.h"


class Server {
 public:
  explicit Server(const ServerConfig& _config, Logger _logger);
  int run();

 private:
  int setup(ServerNetworkingConfig networking_config);

  Game game;
  Logger logger;
  std::vector<DealConfig> dealsToPlay;
};

#endif  // SERVER_H
