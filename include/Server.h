#ifndef SERVER_H
#define SERVER_H

#include <chrono>
#include <memory>
#include <unordered_map>

#include "ConnectionStore.h"
#include "Game.h"
#include "MaybeError.h"

class Server {
 public:
  // Argument timeout is in seconds.
  Server(std::string separator, size_t bufferLen,
         std::chrono::milliseconds timeout);

  void Configure(std::vector<DealConfig> deals_);
  [[nodiscard]] MaybeError Listen(in_port_t port, int maxTcpQueueLen);
  [[nodiscard]] MaybeError Run();

  void EnableDebug();

 private:
  struct Connection {
    // This socketFd will be closed by connectionStore.
    int socketFd;
    std::optional<std::chrono::time_point<std::chrono::system_clock>>
      responseDeadline;
  };

  struct TrickRecord {
    std::array<Card, 4> cards;
    Seat taker;
  };

  struct Points {
    int score;
    int total;
  };

  // On return there are four players playing the game.
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  // Other than that, the method neither receives nor
  // sends messages to players.
  [[nodiscard]] MaybeError SafeUpdate(bool sendState);
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  // Other than that, the method neither receives nor
  // sends messages to players.
  [[nodiscard]] MaybeError Update(bool sendState);

  // Sets responseDeadline.
  [[nodiscard]] MaybeError PushCandidate(int socketFd);
  // Removes the response deadline.
  [[nodiscard]] MaybeError PromoteToPlayer(int socketFd, Seat seat);
  [[nodiscard]] MaybeError PopConnection(int socketFd);

  [[nodiscard]] MaybeError CloseConnection(int socketfd,
                                           std::vector<size_t> &closed);

  [[nodiscard]] std::unique_ptr<Error> ErrorSocket(std::string funName);

  [[nodiscard]] std::unique_ptr<Error> ErrorEmptySeat(std::string funName);

  Game game;
  // In revrese order to the one given in Configure.
  // This way we can use the vectore like a stack.
  std::vector<DealConfig> deals;
  std::vector<TrickRecord> tricks;
  std::array<Points, 4> points;
  // In milliseconds.
  const std::chrono::milliseconds maxTimeout;
  ConnectionStore connectionStore;
  std::vector<Connection> connections;
  std::unordered_map<int, size_t> candidateMap;
  std::unordered_map<int, Seat> playerMap;
  std::array<std::optional<size_t>, 4> seatMap;
  // Input for ConnectionStore::Update.
  ConnectionStore::UpdateData updateArg;
  // Output from ConnectionStore::Update.
  ConnectionStore::UpdateData updateRes;
  // A client hasn't responded to a trick message before timeout.
  bool trickTimeout;
};

#endif  // SERVER_H

