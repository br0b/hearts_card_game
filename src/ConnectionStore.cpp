#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <poll.h>
#include <unistd.h>

#include "ConnectionStore.h"
#include "Logger.h"
#include "MaybeError.h"
#include "MessageBuffer.h"
#include "Utilities.h"

ConnectionStore::~ConnectionStore() {
  for (const pollfd &pfd : pollfds) {
    if (pfd.fd != -1) {
      close(pfd.fd);
    }
  }
}

MaybeError ConnectionStore::Listen(std::optional<in_port_t> port) {
  std::ostringstream oss;
  MaybeError error = std::nullopt;
  Utilities::Socket s = {std::nullopt, port};

  if (error = Utilities::GetBoundSocket(AF_INET6, s); error.has_value()) {
    std::string str = error.value()->GetMessage();
    return error;
  }

  int ret = listen(s.fd.value(), kMaxTCPQueueLength);
  if (ret < 0) {
    return Error::FromErrno("ConnectionStore::Listen");
  }

  pollfds.emplace_back(s.fd.value(), POLLIN, 0);

  if (debugMode) {
    oss << "Listening on port " << s.port.value() << ".";
    Logger::Log(oss.str());
  }

  return std::nullopt;
}

MaybeError ConnectionStore::Update(
    UpdateData &data,
    std::optional<std::chrono::milliseconds> timeout) {
  MaybeError error = std::nullopt;
  time_t pollTimeout = timeout.has_value() ? timeout->count() : -1;

  if (debugMode) {
    std::cerr << "Update. Timeout: " << pollTimeout << " ";
    ReportUpdateData(data);
  }

  if (error = PrePoll(data); error.has_value()) {
    return error;
  }
  
  int pollRes = poll(pollfds.data(), pollfds.size(), pollTimeout);

  if (pollRes < 0) {
    return Error::FromErrno("ConnectionStore::Update");
  }

  if (pollRes > 0) {
    if (error = UpdateBuffers(data); error.has_value()) {
      return error;
    }

    if (error = UpdateListening(data.opened); error.has_value()) {
      return error;
    }
  }

  if (debugMode) {
    ReportUpdateData(data);
  }

  return std::nullopt;
}

void ConnectionStore::EnableDebug() {
  debugMode = true;
}

[[nodiscard]] MaybeError ConnectionStore::Close() {
  std::vector<int> connectionFds;
  UpdateData data;
  MaybeError error;

  close(pollfds.begin()->fd);
  *pollfds.begin() = {-1, 0, 0};

  std::transform(pollfds.begin() + 1, pollfds.end(),
                 std::back_inserter(connectionFds),
                 [](pollfd pfd){ return pfd.fd; });
  for (int fd : connectionFds) {
    if (error = StopReceiving(fd); error.has_value()) {
      return error;
    }
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

MaybeError ConnectionStore::PrePoll(const UpdateData &input) {
  MaybeError error = std::nullopt;

  if (error = PushBuffers(input.msgs); error.has_value()) {
    return error;
  }

  for (int fd : input.closed) {
    if (error = StopReceiving(fd); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError ConnectionStore::PushBuffers(const std::vector<Message> &pending) {
  Client client;

  for (const Message &msg : pending) {
    if (MaybeError error = GetClient(msg.fd, client); error.has_value()) {
      return error;
    }

    pollfds.at(client.pollfdOffset).events |= POLLOUT;
    connections.at(client.messageBufferOffset)->PushMessage(msg.content);
  }

  return std::nullopt;
}

[[nodiscard]] MaybeError ConnectionStore::StopReceiving(int fd) {
  Client client;

  if (MaybeError error = GetClient(fd, client); error.has_value()) {
    return error;
  }

  pollfds.at(client.pollfdOffset).events &= ~POLLIN;
  connections.at(client.messageBufferOffset)->ClearIncoming();
  if (!connections.at(client.messageBufferOffset)->IsOutgoingEmpty()) {
    return std::nullopt;
  }
  return Pop(fd);
}

// TODO: Move nested code to UpdateIncoming and UpdateOutgoing.
MaybeError ConnectionStore::UpdateBuffers(UpdateData &output) {
  MaybeError error = std::nullopt;
  std::optional<std::string> msg;
  output.msgs.clear();
  output.closed.clear();
  // Buffers that have been closed by Update caller
  // either in this or some previous call.
  // They can be safely deleted when they are empty.
  std::vector<int> emptyLingeringBuffers;

  for (size_t i = 0; i < connections.size(); i++) {
    pollfd &pfd = pollfds[i + 1];
    auto &c = connections.at(i);
    if (pfd.revents & (POLLIN | POLLERR)) {
      if (error = c->Receive(); error.has_value()) {
        return error;
      }

      if (!c->IsOpen()) {
        output.closed.push_back(pfd.fd);
        continue;
      } else {
        msg = c->PopMessage();

        if (msg.has_value()) {
          output.msgs.push_back({pfd.fd, std::move(*msg)});
        }
      }
    }

    if (pfd.revents & POLLOUT) {
      if (c->IsOutgoingEmpty()) {
        std::make_unique<Error>("ConnectionStore::UpdateBuffers",
                                "Left over POLLOUT flag");
      } else if (error = c->Send(); error.has_value()) {
        return error;
      }

      if (!c->IsOpen()) {
        output.closed.push_back((size_t)pfd.fd);
      } else if (c->IsOutgoingEmpty()) {
        pfd.events &= ~POLLOUT;
        if (!(pfd.events & POLLIN)) {
          emptyLingeringBuffers.push_back(pfd.fd);
        }
      }
    }
  }

  emptyLingeringBuffers.insert(emptyLingeringBuffers.end(),
                               output.closed.begin(), output.closed.end());
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
  socklen_t addrlen = sizeof(addr);

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

  if (error = Push(fd); error.has_value()) {
    return error;
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
  connections.push_back(std::make_unique<MessageBuffer>(buffer));
  fdMap.emplace(fd, connections.size() - 1);
  return connections.back()->SetSocket(fd);
}

MaybeError ConnectionStore::Pop(int fd) {
  Client client;

  if (MaybeError error = GetClient(fd, client); error.has_value()) {
    return error;
  }

  if (client.messageBufferOffset != connections.size() - 1) {
    // Move last connection to trueIndex.
    pollfds[client.pollfdOffset] = std::move(pollfds.back());
    connections[client.messageBufferOffset] = std::move(connections.back());
    fdMap.at(pollfds.back().fd) = client.messageBufferOffset;
  }

  pollfds.pop_back();
  connections.pop_back();
  fdMap.erase(fd);
  close(fd);
  return std::nullopt;
}

MaybeError ConnectionStore::GetClient(int fd, Client &client) const {
  if (!fdMap.contains(fd)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " not in store.";
    return std::make_unique<Error>("ConnectionStore::GetClient", oss.str());
  }

  client.messageBufferOffset = fdMap.at(fd);
  client.pollfdOffset = client.messageBufferOffset + 1;
  return std::nullopt;
}

void ConnectionStore::ReportUpdateData(const UpdateData &data) const {
  std::ostringstream oss;

  oss<< "Messages: ";
  for (const auto &msg : data.msgs) {
    oss << '(' << msg.fd << ", " << msg.content << "), ";
  }

  oss << "Opened: " << (data.opened.has_value() ?
                        std::to_string(data.opened.value()) + " " : "")
        << "Closed: ";
  
  for (auto c : data.closed) {
    oss << c << ", ";
  }

  Logger::Report(oss.str());
}

