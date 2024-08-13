#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include "ConnectionStore.h"
#include "Game.h"
#include "MaybeError.h"

class Server {
 public:
  Server(const std::string &separator, size_t bufferLen, time_t timeout);

  void Configure(std::vector<DealConfig> deals_);
  [[nodiscard]] MaybeError Listen(in_port_t port, int maxTcpQueueLen);
  [[nodiscard]] MaybeError Run();

 private:
  struct Connection {
    int socketFd;
    std::optional<time_t> messageSentTime;
  };

  struct Points {
    int score;
    int total;
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
  // In revrese order to the one given in Configure.
  // This way we can use the vectore like a stack.
  std::vector<DealConfig> deals;
  std::vector<TrickRecord> tricks;
  std::array<Points, 4> points;
  time_t maxTimeout;
  ConnectionStore connectionStore;
  std::vector<Connection> connections;
  std::unordered_map<int, size_t> candidateMap;
  std::unordered_map<int, size_t> playerMap;
  // Used for ConnectionStore::Update.
  ConnectionStore::UpdateData updateData;
};

#endif  // SERVER_H

