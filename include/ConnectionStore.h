#ifndef CONNECTION_STORE_H
#define CONNECTION_STORE_H

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>

#include "MaybeError.h"
#include "MessageBuffer.h"

class ConnectionStore {
 public:
  struct Message {
    size_t id;
    std::string content;
  };

  struct UpdateData {
    std::vector<Message> msgs;
    std::optional<int> opened;
    std::vector<size_t> closed;
  };

  // A fd can be in both pending and disconnectIndex.
  // Parameter bufferLen is the size of the buffer used for reads and writes.
  ConnectionStore(const std::string &separator, size_t bufferLen);

  ~ConnectionStore();

  [[nodiscard]] MaybeError Listen(in_port_t port, int maxTcpQueueLen);
  // Argument data should contain outgoing messages and fds to close.
  // Return received messages and closed ids in the same argument.
  [[nodiscard]] MaybeError Update(UpdateData &data, time_t timeout);

private:
  [[nodiscard]] MaybeError PrePoll(UpdateData &input);
  [[nodiscard]] MaybeError Send(const std::vector<Message> &pending);
  void StopReceiving(const std::vector<size_t> &fds);
  [[nodiscard]] MaybeError UpdateBuffers(UpdateData &output);
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
  // TODO: Replace with ConvertInput and ConvertOutput.
  [[nodiscard]] MaybeError Convert(UpdateData &data) const;

  // First pollfd is reserved for a listening socket.
  std::vector<pollfd> pollfds;
  std::vector<std::unique_ptr<MessageBuffer>> connections;

  std::string separator;

  // Buffer used for read/write operations.
  std::vector<char> buffer;
  std::unordered_map<int, size_t> fdMap;
};

#endif  // CONNECTION_STORE_H
