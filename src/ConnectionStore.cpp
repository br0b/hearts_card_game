#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <numeric>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <sys/poll.h>
#include <unistd.h>

#include "ConnectionStore.h"
#include "Logger.h"
#include "MaybeError.h"
#include "Utilities.h"

ConnectionStore::ConnectionStore(const std::string &separator, 
                                 size_t bufferLen)
    : separator(separator), buffer(bufferLen) {}

ConnectionStore::~ConnectionStore() {
  for (const pollfd &pfd : pollfds) {
    close(pfd.fd);
  }
}

MaybeError ConnectionStore::Listen(in_port_t port, int maxTcpQueueLen) {
  std::ostringstream oss;
  MaybeError error = std::nullopt;
  Utilities::Socket s = {-1, port};

  if (error = Utilities::GetBoundSocket(AF_INET6, s); error.has_value()) {
    std::string str = error.value()->GetMessage();
    return error;
  }

  int ret = listen(s.fd, maxTcpQueueLen);
  if (ret < 0) {
    return Error::FromErrno("ConnectionStore::Listen");
  }

  // Get actual port.
  if (error = Utilities::GetPortFromFd(s.fd, port); error.has_value()) {
    return error;
  }

  pollfds.emplace_back(s.fd, POLLIN, 0);

  oss << "Listening on port " << port << ".";
  Logger::Log(oss.str());

  return std::nullopt;
}

MaybeError ConnectionStore::Update(
    UpdateData &data,
    std::optional<std::chrono::milliseconds> timeout) {
  MaybeError error = std::nullopt;
  time_t pollTimeout = timeout.has_value() ? timeout->count() : -1;

  if (debugMode) {
    Logger::Report("Update. Timeout: " + std::to_string(pollTimeout));
    ReportUpdateData(data);
  }

  if (error = PrePoll(data); error.has_value()) {
    return error;
  }
  
  int pollRes = poll(&pollfds[0], pollfds.size(), pollTimeout);

  if (pollRes < 0) {
    return Error::FromErrno("ConnectionStore::Update");
  }

  if (pollRes > 0) {
    if (error = UpdateBuffers(data); error.has_value()) {
      return error;
    }

    for (size_t fd : data.closed) {
      if (error = Pop(fd); error.has_value()) {
        return error;
      }
    }

    if (error = UpdateListening(data.opened); error.has_value()) {
      return error;
    }
  }

  if (debugMode) {
    Logger::Report("Update results");
    ReportUpdateData(data);
  }

  return std::nullopt;
}

void ConnectionStore::EnableDebug() {
  debugMode = true;
}

[[nodiscard]] MaybeError ConnectionStore::Close() {
  std::vector<size_t> allIds(connections.size());
  UpdateData data;
  MaybeError error;

  close(pollfds.begin()->fd);
  pollfds.at(0) = {-1, 0, 0};
  std::iota(allIds.begin(), allIds.end(), 0);
  if (error = StopReceiving(allIds); error.has_value()) {
    return error;
  }

  while (!connections.empty()) {
    if (error = Update(data, std::nullopt); error.has_value()) {
      return error;
    }

    if (!data.msgs.empty() || data.opened.has_value()
        || !data.closed.empty()) {
      std::ostringstream oss;
      oss << "Changes shouldn't happen after Close.";
      return std::make_unique<Error>("ConnectionStore::Close", oss.str());
    }
  }

  return std::nullopt;
}

bool ConnectionStore::IsEmpty() const {
  return connections.empty();
}

MaybeError ConnectionStore::PrePoll(UpdateData &input) {
  MaybeError error = std::nullopt;

  if (error = Convert(input); error.has_value()) {
    return error;
  }

  if (error = PushBuffers(input.msgs); error.has_value()) {
    return error;
  }

  if (error = StopReceiving(input.closed); error.has_value()) {
    return error;
  }

  return std::nullopt;
}

MaybeError ConnectionStore::PushBuffers(
      const std::vector<Message> &pending) {
  MaybeError error = std::nullopt;

  for (const Message &msg : pending) {
    pollfds[msg.id + 1].events |= POLLOUT;
    connections[msg.id]->PushMessage(msg.content);
  }

  return std::nullopt;
}

MaybeError ConnectionStore::StopReceiving(const std::vector<size_t> &ids) {
  MaybeError error;

  for (size_t id : ids) {
    pollfds[id + 1].events &= ~POLLIN;
    connections[id]->ClearIncoming();
    if (!connections[id]->IsEmpty()) {
      continue;
    }
    if (error = Pop(pollfds[id + 1].fd); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

// TODO: Move nested code to UpdateIncoming and UpdateOutgoing.
MaybeError ConnectionStore::UpdateBuffers(UpdateData &output) {
  MaybeError error = std::nullopt;
  MessageBuffer::Result res;
  std::optional<std::string> msg;
  output.msgs.clear();
  output.closed.clear();
  // Buffers that have been closed by Update caller
  // either in this or some previous call.
  // They can be safely deleted when they are empty.
  std::vector<int> emptyLingeringBuffers;

  for (size_t i = 0; i < connections.size(); i++) {
    pollfd &pfd = pollfds[i + 1];
    if (pfd.revents & (POLLIN | POLLERR)) {
      if (error = connections[i]->Receive(res); error.has_value()) {
        return error;
      }

      if (res.IsClosed()) {
        output.closed.push_back((size_t)pfd.fd);
        continue;
      } else {
        if (error = res.GetMessage(msg); error.has_value()) {
          return error;
        }

        if (msg.has_value()) {
          output.msgs.push_back({(size_t)pfd.fd, std::move(*msg)});
        }
      }
    }

    if (pfd.revents & POLLOUT) {
      if (error = connections[i]->Send(res); error.has_value()) {
        return error;
      }

      if (res.IsClosed()) {
        output.closed.push_back((size_t)pfd.fd);
      } else if (connections[i]->IsEmpty()) {
        pfd.events &= ~POLLOUT;
        if (!(pfd.events & POLLIN)) {
          emptyLingeringBuffers.push_back(pfd.fd);
        }
      }
    }
  }

  for (int fd : emptyLingeringBuffers) {
    if (error = Pop(fd); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError ConnectionStore::UpdateListening(std::optional<int> &opened) {
  MaybeError error;

  opened.reset();

  if (!(pollfds[0].revents & POLLIN)) {
    return std::nullopt;
  }

  sockaddr_storage addr;
  socklen_t addrlen;

  int fd = accept(pollfds[0].fd, (sockaddr *)&addr, &addrlen);
  if (fd < 0) {
    return Error::FromErrno("ConnectionStore::UpdateListening");
  }
  
  if (addrlen > sizeof(addr)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " has size bigger than sockaddr_storage";
    return std::make_unique<Error>("ConnectionStore::UpdateListening",
                                   oss.str());
  }

  if (fcntl(fd, F_SETFD, O_NONBLOCK) != 0) {
    return Error::FromErrno("ConnectionStore::UpdateListening");
  }

  if (error = Push(fd); error.has_value()) {
    return error;
  }

  if (debugMode) {
    std::optional<std::string> addr = connections.at(fdMap[fd])->GetRemote();
    if (!addr.has_value()) {
      return std::make_unique<Error>("ConnectionStore::UpdateListening",
                                     "No remote address.");
      return error;
    }
    std::ostringstream oss;
    oss << "New connection (fd: "<< fd << ", addr: " << addr.value() << ')';
  }

  opened = fd;
  return std::nullopt;
}

MaybeError ConnectionStore::Push(int fd) {
  if (fdMap.contains(fd)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " already in store.";
    return std::make_unique<Error>("ConnectionStore::Push", oss.str());
  }

  pollfds.emplace_back(fd, POLLIN, 0);
  connections.push_back(std::make_unique<MessageBuffer>(buffer, separator));
  fdMap.emplace(fd, connections.size() - 1);
  return connections.back()->SetSocket(fd);
}

MaybeError ConnectionStore::Pop(int fd) {
  size_t id = 0;
  if (MaybeError error = GetId(fd, id); error.has_value()) {
    return error;
  }

  if (id != connections.size() - 1) {
    // Move last connection to trueIndex.
    pollfds[id + 1] = std::move(pollfds.back());
    connections[id] = std::move(connections.back());
    fdMap.at(pollfds.back().fd) = id;
  }

  pollfds.pop_back();
  connections.pop_back();
  fdMap.erase(fd);
  close(fd);
  return std::nullopt;
}

MaybeError ConnectionStore::GetId(int fd, size_t &id) const {
  if (!fdMap.contains(fd)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " not in store.";
    return std::make_unique<Error>("ConnectionStore::GetId", oss.str());
  }

  id = fdMap.at(fd);
  return std::nullopt;
}

MaybeError ConnectionStore::Convert(UpdateData &data) const {
  MaybeError error = std::nullopt;

  for (Message &msg : data.msgs) {
    if (error = GetId(msg.id, msg.id); error.has_value()) {
      return error;
    }
  }

  for (size_t &id: data.closed) {
    if (error = GetId(id, id); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

void ConnectionStore::ReportUpdateData(const UpdateData &data) const {
  std::ostringstream oss;

  oss<< "Messages: ";
  for (const auto &msg : data.msgs) {
    oss << '(' << msg.id << ", " << msg.content << "), ";
  }

  oss << "Opened: " << (data.opened.has_value() ?
                        std::to_string(data.opened.value()) + " " : "")
        << "Closed: ";
  
  for (auto c : data.closed) {
    oss << c << ", ";
  }

  Logger::Report(oss.str());
}

