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
  template <typename TId>
  struct Message {
    TId id;
    std::string content;
  };

  // When as an argument: a connection can be in closed or msgs or both.
  // When returned: a connection can't be only in msgs xor opened xor closed.
  template <typename TId>
  struct UpdateData {
    std::vector<Message<TId>> msgs;
    std::optional<int> opened;
    std::vector<TId> closed;
  };

  ~ConnectionStore();

  [[nodiscard]] MaybeError Listen(std::optional<in_port_t> port);
  // Argument data should contain outgoing messages and fds to close.
  // Return received messages and closed ids in the same argument.
  // Don't return the same socket fds as on input.
  [[nodiscard]] MaybeError Update(
      UpdateData<int> &data,
      std::optional<std::chrono::milliseconds> timeout);

  void EnableDebug();
  // Closes all connections.
  // Ensures all the outgoing messages are written to the TCP buffers.
  [[nodiscard]] MaybeError Close();

  [[nodiscard]] bool IsEmpty() const;

private:
  [[nodiscard]] MaybeError PrePoll(UpdateData<int> &input);
  [[nodiscard]] MaybeError PushBuffers(const std::vector<Message<size_t>> &pending);
  [[nodiscard]] MaybeError StopReceiving(const std::vector<size_t> &fds);
  [[nodiscard]] MaybeError UpdateBuffers(UpdateData<int> &output);
  // Put new client fd in opened.
  [[nodiscard]] MaybeError UpdateListening(std::optional<int> &opened);

  [[nodiscard]] MaybeError Push(int fd);
  // Close the fd.
  [[nodiscard]] MaybeError Pop(int fd);

  // +++ TIMEOUT POLICY +++
  // When called, timeout should specify the desired max timeout.
  // Returns the true timeout in the same argument.
  // The returned timeout is maxTimeout or
  // maxTimeout + min(timeoutStarts) - currentTime (1),
  // whichever is smaller.
  // If there exists such a timeoutStart that value (1) is negative, then
  // return error.

  // +++ pollfd = index + 1 +++

  [[nodiscard]] MaybeError GetId(int fd, size_t &id) const;
  // Argument src is invalidaded.
  [[nodiscard]] MaybeError Convert(UpdateData<int> &src,
                                   UpdateData<size_t> &dst) const;

  void ReportUpdateData(const UpdateData<int> &data) const;

  // First pollfd is reserved for a listening socket.
  std::vector<pollfd> pollfds;
  std::vector<std::unique_ptr<MessageBuffer>> connections;

  static constexpr int kMaxTCPQueueLength = 128;

  // Buffer used for read/write operations.
  std::vector<char> buffer = std::vector<char>(4096);
  std::unordered_map<int, size_t> fdMap;
  bool debugMode;
};

#endif  // CONNECTION_STORE_H
