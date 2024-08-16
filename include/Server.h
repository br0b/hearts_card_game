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
  typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

  class Deadline {
   public:
    // Set to ms milliseconds into the future.
    void Set(std::chrono::milliseconds ms);

    [[nodiscard]] std::chrono::milliseconds GetTimeLeft() const;
    [[nodiscard]] bool IsOverdue() const;

   private:
    TimePoint deadline;
  };

  struct Connection {
    // This socketFd will be closed by connectionStore.
    int socketFd;
    std::optional<Deadline> responseDeadline;
  };

  struct TrickRecord {
    std::array<Card, 4> cards;
    Seat taker;
  };

  class Points {
   public:
    [[nodiscard]] MaybeError Add(int x);

    [[nodiscard]] int GetScore() const;
    [[nodiscard]] int GetTotal() const;
   private:
    int score;
    int total;
  };

  // On return there are four players playing the game,
  // i.e. all fields of seatMap contain a value.
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  [[nodiscard]] MaybeError SafeUpdate(bool sendState);
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  // Other than that, the method neither receives nor
  // sends messages to players.
  [[nodiscard]] MaybeError Update(bool sendState);

  // Assumes that updateRes.msgs contains at most a single message from
  // each player. Assumes the game is ongoing.
  [[nodiscard]] MaybeError HandlePlayerMessages();

  // Sets responseDeadline.
  [[nodiscard]] MaybeError PushCandidate(int socketFd);
  // Removes the response deadline.
  [[nodiscard]] MaybeError PromoteToPlayer(int socketFd, Seat seat);
  [[nodiscard]] MaybeError PopConnection(int socketFd);
  // Pushes socketFd to updateArg.closed.
  [[nodiscard]] MaybeError CloseConnection(int socketFd);

  // Calls Game::Deal and sends deal messages to all players.
  [[nodiscard]] MaybeError NewDeal();

  // Send current trick to the active player and set their response deadline.
  [[nodiscard]] MaybeError SendTrick();

  [[nodiscard]] MaybeError ResetResponseDeadline(Seat seat);
  [[nodiscard]] MaybeError IsResponseOverdue(Seat seat, bool &isOverdue) const;

  [[nodiscard]] MaybeError GetPlayerFd(Seat seat, int &socketFd) const;

  [[nodiscard]] static std::unique_ptr<Error> ErrorSocket(std::string funName);

  [[nodiscard]] static std::unique_ptr<Error> ErrorEmptySeat(std::string funName);

  Game game;
  int nPlayers = 0;
  // In revrese order to the one given in Configure.
  // This way we can use the vectore like a stack.
  std::vector<DealConfig> deals;
  std::vector<TrickRecord> tricks;
  std::array<Points, 4> points;
  // In milliseconds.
  const std::chrono::milliseconds timeout;
  ConnectionStore connectionStore;
  std::vector<Connection> connections;
  std::unordered_map<int, size_t> candidateMap;
  std::unordered_map<int, Seat> playerMap;
  std::array<std::optional<size_t>, 4> seatMap;
  // Input for ConnectionStore::Update.
  ConnectionStore::UpdateData updateArg;
  // Output from ConnectionStore::Update.
  ConnectionStore::UpdateData updateRes;
};

#endif  // SERVER_H

