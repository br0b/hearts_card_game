#ifndef CLIENT_H
#define CLIENT_H

#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <optional>

#include "ConnectionProtocol.h"
#include "MessageBuffer.h"
#include "MaybeError.h"
#include "Seat.h"

class Client {
 public:
  // Argument bufferLen specifies the size of the buffer used by Client for
  // reads and writes.
  Client(Seat seat, size_t bufferLen, std::string separator);

  [[nodiscard]] MaybeError Connect(
      std::string host,
      uint16_t port,
      std::optional<ConnectionProtocol> protocol);

  // Client must be connected first.
  [[nodiscard]] MaybeError Run(bool isAutomatic);

 private:
  // The return address family will be used with getaddrinfo.
  [[nodiscard]] static int GetAddressFamily(
      std::optional<ConnectionProtocol> protocol);

  Seat seat;
  std::vector<char> buffer;
  MessageBuffer server;
  std::unique_ptr<MessageBuffer> user;
  // The first field is the server and the second is stdin.
  std::vector<pollfd> pollfds;
};

#endif //CLIENT_H
