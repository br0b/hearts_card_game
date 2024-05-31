//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef SERVER_H
#define SERVER_H

#include <poll.h>

#include "Connection.h"
#include "Game.h"
#include "Port.h"
#include "ServerConfig.h"

class Server {
 public:
  explicit Server(const ServerConfig& _config);
  ~Server();
  int run();

 private:
  std::optional<Error> setup(ServerNetworkingConfig networking_config);
  std::optional<Error> setupPollfds(Port port);
  static std::variant<int, Error> getListeningSocket(Port port);
  static std::optional<Error> acceptBothIPv4AndIPv6(int socketfd);
  static std::optional<Error> bindSocket(int socketfd, Port port);

  Game game;
  std::vector<DealConfig> dealsToPlay;
  // At least 9 connections have to be supported.
  const int kTimeout;
  static constexpr int kNConnections = 32;
  static constexpr int kListenBacklog = 32;
  // 0: accepting connections
  // 1-4: players
  // 5-...: serving player candidates
  std::array<pollfd, kNConnections> pollfds;
  std::unordered_map<int, Connection> connections;
};

#endif  // SERVER_H
