//
// Created by robert-grigoryan on 5/30/24.
//

#include <utility>

#include "../include/Server.h"

Server::Server(const ServerConfig& _config, Logger _logger)
      : logger(_logger), dealsToPlay(_config.getDeals()) {
  setup(_config.getNetworkingConfig());
}

int Server::setup(ServerNetworkingConfig networking_config) {
  // TODO
}

int Server::run() { return 0; }
