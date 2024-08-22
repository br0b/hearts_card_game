#ifndef CONNECTION_STORE_H
#define CONNECTION_STORE_H

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <chrono>
#include <unordered_map>
#include <vector>

#include "MaybeError.h"
#include "MessageBuffer.h"

class ConnectionStore {
 public:
  struct Message {
    int fd;
    std::string content;
  };

  // When as an argument: a connection can be in closed or msgs or both.
  // When returned: a connection can't be only in msgs xor opened xor closed.
  struct UpdateData {
    std::vector<Message> msgs;
    std::optional<int> opened;
    std::vector<int> closed;
  };

  ~ConnectionStore();

  [[nodiscard]] MaybeError Listen(std::optional<in_port_t> port);
  // Argument data should contain outgoing messages and fds to close.
  // Return received messages and closed ids in the same argument.
  // Don't return the same socket fds as on input.
  [[nodiscard]] MaybeError Update(
      UpdateData &data,
      std::optional<std::chrono::milliseconds> timeout);

  void EnableDebug();
  // Closes all connections.
  // Ensures all the outgoing messages are written to the TCP buffers.
  [[nodiscard]] MaybeError Close();

  [[nodiscard]] bool IsEmpty() const;

private:
  struct Client {
    size_t pollfdOffset;
    size_t messageBufferOffset;
  };

  [[nodiscard]] MaybeError PrePoll(const UpdateData &input);
  [[nodiscard]] MaybeError PushBuffers(const std::vector<Message> &pending);
  [[nodiscard]] MaybeError StopReceiving(int fd);
  [[nodiscard]] MaybeError UpdateBuffers(UpdateData &output);
  // Put new client fd in opened.
  [[nodiscard]] MaybeError UpdateListening(std::optional<int> &opened);

  [[nodiscard]] MaybeError Push(int fd);
  // Close the fd.
  [[nodiscard]] MaybeError Pop(int fd);

  [[nodiscard]] MaybeError GetClient(int fd, Client &client) const;

  void ReportUpdateData(const UpdateData &data) const;

  // First pollfd is reserved for a listening socket.
  std::vector<pollfd> pollfds;
  std::vector<std::unique_ptr<MessageBuffer>> connections;

  static constexpr int kMaxTCPQueueLength = 128;

  // Buffer used for read/write operations.
  std::array<char, 4096> buffer;
  std::unordered_map<int, size_t> fdMap;
  bool debugMode = false;
};

#endif  // CONNECTION_STORE_H
