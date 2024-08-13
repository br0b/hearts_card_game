#include <cstddef>
#include <cstring>
#include <optional>
#include <sstream>
#include <fcntl.h>
#include <netinet/in.h>
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

  if (error = Utilities::GetBoundSocket(AF_INET6, s);
      error.has_value()) {
    return error;
  }

  if (listen(s.fd, maxTcpQueueLen) < 0) {
    return Error::FromErrno("listen");
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

MaybeError ConnectionStore::Update(UpdateData &data, time_t timeout) {
  MaybeError error = std::nullopt;

  if (error = PrePoll(data); error.has_value()) {
    return error;
  }
  
  int pollRes = poll(&pollfds[0], pollfds.size(), timeout);

  if (pollRes < 0) {
    return Error::FromErrno("poll");
  }

  if (pollRes > 0) {
    if (error = UpdateBuffers(data); error.has_value()) {
      return error;
    }

    if (error = UpdateListening(data.opened); error.has_value()) {
      return error;
    }
  }

  return std::nullopt;
}

MaybeError ConnectionStore::PrePoll(UpdateData &input) {
  MaybeError error = std::nullopt;

  if (error = Convert(input); error.has_value()) {
    return error;
  }

  if (error = Send(input.msgs); error.has_value()) {
    return error;
  }

  StopReceiving(input.closed); error.has_value();

  return std::nullopt;
}

MaybeError ConnectionStore::Send(
      const std::vector<Message> &pending) {
  MaybeError error = std::nullopt;

  for (const Message &msg : pending) {
    pollfds[msg.id + 1].events |= POLLOUT;
    connections[msg.id]->PushMessage(msg.content);
  }

  return std::nullopt;
}

void ConnectionStore::StopReceiving(const std::vector<size_t> &ids) {
  for (size_t id : ids) {
    pollfds[id + 1].events &= ~POLLIN;
    connections[id]->ClearIncoming();
  }
}

// TODO: Move nested code to ConnectionStore::UpdateBuffer.
MaybeError ConnectionStore::UpdateBuffers(UpdateData &output) {
  std::string message = "";
  MaybeError error = std::nullopt;
  output.msgs.clear();
  output.closed.clear();

  for (size_t i = 0; i < connections.size(); i++) {
    pollfd &pfd = pollfds[i + 1];
    if (pfd.revents & (POLLIN | POLLERR)) {
      if (error = connections[i]->Receive(); error.has_value()) {
        return error;
      }

      if (!connections[i]->IsOpen()) {
        output.closed.push_back(i);
      }
      
      if (connections[i]->ContainsMessage()) {
        if (error = connections[i]->PopMessage(message); error.has_value()) {
          return error;
        }

        output.msgs.push_back({(size_t)pfd.fd, std::move(message)});
      }
    }

    if (pfd.revents & POLLOUT) {
      if (error = connections[i]->Send(); error.has_value()) {
        return error;
      }

      if (!connections[i]->IsOpen()) {
        output.closed.push_back(i);
      }
      
      if (!(pfd.events & POLLIN) && connections[i]->IsEmpty()) {
        if (error = Pop(pfd.fd); error.has_value()) { 
          return error;
        }
      }
    }
  }

  // TODO: Use StopReceiving and ConvertOutput outside of function.
  for (size_t &id : output.closed) {
    pollfds[id + 1].events &= ~POLLIN;
    connections[id]->ClearIncoming();
    id = pollfds[id + 1].fd;
  }

  return std::nullopt;
}

MaybeError ConnectionStore::UpdateListening(std::optional<int> &opened) {
  MaybeError error = std::nullopt;

  if (!(pollfds[0].revents & POLLIN)) {
    opened.reset();
    return std::nullopt;
  }

  sockaddr_storage addr;
  socklen_t addrlen;

  int fd = accept(pollfds[0].fd, (sockaddr *)&addr, &addrlen);
  if (fd < 0) {
    return Error::FromErrno("accept");
  }
  
  if (addrlen > sizeof(addr)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " has size bigger than sockaddr_storage";
    return std::make_unique<Error>("ConnectionStore::UpdateListening", oss.str());
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

  fdMap.emplace(fd, connections.size());
  pollfds.emplace_back(fd, POLLIN, 0);
  connections.push_back(std::make_unique<MessageBuffer>(buffer, separator));
  return connections.back()->SetSocket(fd);
}

MaybeError ConnectionStore::Pop(int fd) {
  MaybeError error = std::nullopt;
  size_t id = 0;

  if (error = GetId(fd, id); error.has_value()) {
    return error;
  }

  close(fd);

  if (connections.size() > 1) {
    // Move last connection to trueIndex.
    pollfds[id + 1] = std::move(pollfds.back());
    connections[id] = std::move(connections.back());
    fdMap[connections.size() - 1] = id;
  }
  pollfds.pop_back();
  connections.pop_back();
  fdMap.erase(connections.size() - 1);

  return std::nullopt;
}

MaybeError ConnectionStore::GetId(int fd, size_t &id) const {
  if (!fdMap.contains(fd)) {
    std::ostringstream oss;
    oss << "Socket " << fd << " not in store.";
    return std::make_unique<Error>("ConnectionStore::Pop", oss.str());
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

