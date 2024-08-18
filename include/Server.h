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
  typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

  Server();

  void Configure(std::vector<DealConfig> deals_,
                 std::optional<std::chrono::seconds> maxTimeout_);
  [[nodiscard]] MaybeError Listen(std::optional<in_port_t> port);
  [[nodiscard]] MaybeError Run();

  void EnableDebug();

 private:
  class Deadline {
   public:
    Deadline(std::chrono::milliseconds timeout);

    // Set to ms milliseconds into the future.
    void Reset();
    void Set(TimePoint timePoint);

    [[nodiscard]] TimePoint Get() const;
    [[nodiscard]] std::chrono::milliseconds GetTimeLeft() const;
    [[nodiscard]] bool IsOverdue() const;

   private:
    TimePoint deadline;
    const std::chrono::milliseconds timeout;
  };

  class Player {
   public:
    Player(Seat seat);

    void AddPoints(int x);
    void ResetScore();
    void SetFd(std::optional<int> fd_);

    [[nodiscard]] int GetScore() const;
    [[nodiscard]] int GetTotal() const;
    [[nodiscard]] std::optional<int> GetFd() const;
    [[nodiscard]] Seat GetSeat() const;

   private:
    int score;
    int total;
    // Might not be connected at the moment.
    std::optional<int> fd;
    const Seat seat;
  };

  struct Candidate {
    int fd;
    Deadline deadline;
  };

  struct TrickRecord {
    std::array<Card, 4> cards;
    Seat taker;
  };

  // On return there are four players playing the game,
  // i.e. all fields of seatMap contain a value.
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  [[nodiscard]] MaybeError SafeUpdate();
  // If sendState is true, then send the state of the game after
  // a connection becomes a player.
  [[nodiscard]] MaybeError Update();

  // Assumes the game is ongoing. On return places pending messages and kicked players to data.
  [[nodiscard]] MaybeError HandlePlayerMessages();

  // Sets responseDeadline.
  [[nodiscard]] MaybeError PushCandidate(int socketFd);
  // Removes the response deadline.
  [[nodiscard]] MaybeError PromoteToPlayer(int socketFd, Seat seat);
  [[nodiscard]] MaybeError PopConnection(int socketFd);
  // Pushes socketFd to updateArg.closed and calls PopConnection.
  [[nodiscard]] MaybeError CloseConnection(int socketFd);

  // Calls Game::Deal and sends deal messages to all players.
  [[nodiscard]] MaybeError NewDeal();

  // Send current trick to the active player and set their response deadline.
  [[nodiscard]] MaybeError SendTrick();

  [[nodiscard]] static std::unique_ptr<Error> ErrorSocket(std::string funName);
  [[nodiscard]] static std::unique_ptr<Error> ErrorEmptySeat(std::string funName);

  Game game;
  // In revrese order to the one given in Configure.
  // This way we can use the vectore as a stack.
  std::vector<DealConfig> deals;
  std::vector<TrickRecord> tricks;
  // In milliseconds.
  std::chrono::milliseconds maxTimeout{5000};
  ConnectionStore connectionStore;
  std::vector<Candidate> candidates;
  // Map fds to an index to deadlines.
  std::unordered_map<int, size_t> candidateMap;
  std::array<Player, 4> players{{{Seat::Value::kN}, {Seat::Value::kE},
                                 {Seat::Value::kS}, {Seat::Value::kE}}};
  std::unordered_map<int, Seat> playerMap;
  ConnectionStore::UpdateData<int> updateData;
  // Received
  std::vector<ConnectionStore::Message<Seat>> playerMessages;
  Deadline trickDeadline;
};

#endif  // SERVER_H
