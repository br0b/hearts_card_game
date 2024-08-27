#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include <netdb.h>
#include <poll.h>
#include <optional>
#include <unistd.h>
#include <unordered_set>

#include "Card.h"
#include "ConnectionProtocol.h"
#include "Message.h"
#include "MessageBuffer.h"
#include "MaybeError.h"
#include "Seat.h"
#include "TrickNumber.h"

class Client {
 public:
  // Argument bufferLen specifies the size of the buffer used by Client for
  // reads and writes.
  Client(Seat seat);

  [[nodiscard]] MaybeError Connect(
      std::string host,
      uint16_t port,
      std::optional<ConnectionProtocol> protocol);

  // Client must be connected first.
  [[nodiscard]] MaybeError Run(bool isAutomatic);

 private:
  void PrintCards() const;
  void PrintUserStr(const Message *msg) const;
  void PrintHelp() const;
  [[nodiscard]] MaybeError HandleServerMessage(std::string msg);
  // Doesn't modify player cards.
  [[nodiscard]] MaybeError PlayTrick(Card card);

  // The return address family will be used with getaddrinfo.
  [[nodiscard]] static int GetAddressFamily(
      std::optional<ConnectionProtocol> protocol);

  [[nodiscard]] std::unique_ptr<Error> ErrorDeserialize(std::string funName,
                                                        std::string msg);

  Seat seat;
  std::array<char, 4096> buffer;
  MessageBuffer server{buffer};
  MessageBuffer user{buffer};
  // The first field is the server and the second is stdin.
  std::array<pollfd, 2> pollfds{{{-1, 0, 0}, {-1, 0, 0}}};
  std::array<std::unordered_set<Card>, 4> cards;
  std::vector<Card> taken;
  int nPointMessagesReceived = 0;
  std::optional<TrickNumber> nextTrickNumber;
  bool isUserTrickNeeded = false;
  std::optional<Card> playerInput;
  static constexpr size_t kServerId = 0;
  static constexpr size_t kUserId = 1;
};

#endif //CLIENT_H
