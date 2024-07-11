//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef SERVER_H
#define SERVER_H

#include <memory>

#include "Connection.h"
#include "Game.h"
#include "PortConfig.h"
#include "ServerConfig.h"
#include "ErrorCritical.h"
#include "MessageTrick.h"

class Server {
 public:
  static std::variant<std::unique_ptr<Server>, Error> Create(
      const ServerConfig& config);
  [[nodiscard]] std::optional<Error> run();
  ~Server();

 private:
  [[nodiscard]] std::optional<Error> initGame();
  [[nodiscard]] std::optional<Error> playGame();

  // Where this method returns, all 4 players are gathered.
  [[nodiscard]] std::optional<Error> updateEvents();
  [[nodiscard]] std::optional<Error> updateListeningSocket();
  [[nodiscard]] std::optional<Error> setupPollfds(PortConfig port);
  void setupListeningPollfdEvents();
  static std::variant<int, Error> getListeningSocket(PortConfig port);
  static std::optional<Error> acceptBothIPv4AndIPv6(int socketfd);
  static std::optional<Error> bindSocket(int socketfd, PortConfig port);
  // Store seconds in int, because poll takes timeout as int.
  [[nodiscard]] std::optional<int> getTimeout();
  [[nodiscard]] std::optional<int> getPollfdForCandidate() const;
  [[nodiscard]] std::optional<Error> addCandidate(int pollfdIndex);
  void rejectCandidate(int pollfdIndex);
  [[nodiscard]] std::optional<Error> updateCandidateBuffers(
      int pollfdIndex) const;
  [[nodiscard]] std::optional<Error> handleCandidateMessage(int pollfdIndex);
  [[nodiscard]] std::optional<Error> handlePlayerEvents();
  [[nodiscard]] std::optional<Error> handlePlayerMessage(Seat player);
  [[nodiscard]] std::optional<Error> callPoll();
  [[nodiscard]] std::optional<Error> updatePlayers();
  [[nodiscard]] std::optional<Error> updateCandidates();
  [[nodiscard]] std::optional<Error> updateCandidate(int pollfdIndex);
  void clearRevents();
  void logPollEvents();
  void logPollRevents();

  [[nodiscard]] std::optional<Error> addPlayer(Seat::Position position,
                                               int pollfdIndexCandidate);
  [[nodiscard]] std::optional<ErrorCritical> removePlayer(
      Seat::Position position);

  void setPort(const sockaddr_in6& address);
  [[nodiscard]] std::optional<Error> setAddressStr(sockaddr_in6 addr);

  std::variant<std::unique_ptr<Message>, Error> receiveMessage(
      int pollfdIndex) const;
  void sendMessage(int pollfdIndex, const Message& message) const;
  std::optional<Error> sendTrickToCurrentPlayer() const;
  std::optional<Error> playCard(const MessageTrick& trick);

  void reportReceivedMessage(int pollfdIndex, const Message& message) const;
  void reportSentMessage(int pollfdIndex, const Message& message) const;

  explicit Server(const ServerConfig& _config);

  uint16_t port;
  std::string address;
  Game game;
  std::vector<DealConfig> dealsToPlay;
  // True if all the players have gathered at least once.
  bool isGameInitiated;
  int nCandidates;
  int nCurrentPlayers;
  const int kMaxNCandidates;
  const int kTimeout;
  static constexpr int kNPlayers = 4;
  static constexpr int kListeningIndex = 0;
  // At least 9 connections have to be supported.
  static constexpr int kNConnections = 32;
  static constexpr int kListenBacklog = 32;
  // 0: accepting connections
  // 1-4: players
  // 5-...: serving player candidates
  std::array<pollfd, kNConnections> pollfds;
  std::array<std::optional<time_t>, kNConnections> waitingSince;
  std::unordered_map<int, std::unique_ptr<Connection>> connections;
  std::unordered_set<int> rejectedCandidates;
};

#endif  // SERVER_H
