//
// Created by robert-grigoryan on 5/31/24.
//
#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <memory>
#include <optional>
#include <variant>

#include "Error.h"
#include "Message.h"

class Connection {
public:
  std::variant<std::unique_ptr<Connection>, Error> static Accept(
      pollfd* _pfd, std::optional<time_t>& _waitingSince, int listeningfd);
  // Assumes no countdown is in progress.
  [[nodiscard]] std::optional<Error> beginTimeout() const;
  [[nodiscard]] std::optional<double> getTimeSinceTimeout() const;
  void clearTimeout() const;
  void setPollfd(pollfd* _pfd);
  void resetPollfd() const;
  [[nodiscard]] std::optional<Error> updateBuffers();
  [[nodiscard]] bool hasMessage() const;
  [[nodiscard]] std::variant<std::unique_ptr<Message>, Error> popMessage();
  void sendMessage(const Message& message);
  [[nodiscard]] bool isOutgoingBufferEmpty() const;
  [[nodiscard]] std::string getAddressStr();
  [[nodiscard]] std::string getServerAddressStr();
  ~Connection();

private:
  [[nodiscard]] std::variant<sockaddr_in6, Error> acceptConnection(
      int listeningfd) const;
  [[nodiscard]] std::optional<Error> setNonblocking() const;
  [[nodiscard]] std::optional<Error> updateIncoming();
  [[nodiscard]] std::optional<Error> updateOutgoing();
  Connection(pollfd* _pfd, std::optional<time_t>& _timeSinceTimeout);

  std::string incoming;
  std::string outgoing;
  pollfd* pfd;
  std::string addressClient;
  std::string addressServer;
  std::optional<time_t>& timeSinceTimeout;
  // Buffer used for read/write operations.
  char* buffer;
  static constexpr size_t kBufferSize = 1024;
};

#endif  // CONNECTION_H
