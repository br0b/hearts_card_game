#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include "ConnectionStore.h"
#include "Game.h"
#include "MaybeError.h"
#include "Points.h"

class Server {
 public:
  Server(const std::string &separator, size_t bufferLen, time_t timeout);

  ~Server();

  void Configure(std::unique_ptr<std::vector<DealConfig>> deals_);
  [[nodiscard]] MaybeError Listen(in_port_t port, int maxTcpQueueLen);
  [[nodiscard]] MaybeError Run();

 private:
  struct Connection {
    int socketFd;
    std::optional<time_t> messageSentTime;
  };

  struct TrickRecord {
    std::array<Card, 4> cards;
    Seat taker;
  };

  [[nodiscard]] MaybeError InitGame();
  [[nodiscard]] MaybeError PlayGame();

  [[nodiscard]] MaybeError PushConnection(int socketFd);
  [[nodiscard]] MaybeError PopConnection(int socketFd);

  Game game;
  std::unique_ptr<std::vector<DealConfig>> deals;
  Points score;
  Points total;
  std::vector<TrickRecord> tricks;
  time_t maxTimeout;
  ConnectionStore connectionStore;
  // First four positions are reserved for players. The rest is for candidates.
  // These can only be closed by the ConnectionStore which gave them.
  std::vector<Connection> connections;
  // Length of four. 0 - North, 1 - East, ...
  std::vector<size_t> players;
  std::unordered_map<int, size_t> candidateMap;
  // Used for ConnectionStore::Update.
  ConnectionStore::UpdateData updateData;
};

#endif  // SERVER_H

