#include <arpa/inet.h>
#include <cstring>
#include "MaybeError.h"
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Server.h"

Server::Server(const std::string &separator, size_t bufferLen, time_t timeout)
    : maxTimeout(timeout), connectionStore(separator, bufferLen), players(4) {}

void Server::Configure(std::unique_ptr<std::vector<DealConfig>> deals_) {
  signal(SIGPIPE, SIG_IGN);
  deals = std::move(deals_);
}

MaybeError Server::Listen(in_port_t port, int maxTcpQueueLen) {
  return connectionStore.Listen(port, maxTcpQueueLen);
}

// TODO: Implement.
MaybeError Server::Run() {
  return std::nullopt;
}

