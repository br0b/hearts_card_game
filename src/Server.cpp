//
// Created by robert-grigoryan on 5/30/24.
//
#include <netdb.h>
#include <netinet/in.h>  // For sockaddr_in, sockaddr_in6
#include <sys/socket.h>  // For socket functions
#include <unistd.h>

#include <csignal>
#include <cstring>

#include "Logger.h"
#include "Server.h"

Server::Server(const ServerConfig& _config)
    : dealsToPlay(_config.getDeals()),
      kTimeout(_config.getNetworkingConfig().getTimeout()),
      pollfds() {
  setup(_config.getNetworkingConfig());
}

Server::~Server() {
  for (auto& [fd, events, revents] : pollfds) {
    if (fd != -1) {
      close(fd);
    }
  }
}

std::optional<Error> Server::setup(
    const ServerNetworkingConfig networking_config) {
  Logger::log("Setting up server with config: " + networking_config.toString());

  signal(SIGPIPE, SIG_IGN);
  return setupPollfds(networking_config.getPort());
}

std::optional<Error> Server::setupPollfds(const Port port) {
  std::variant<int, Error> listeningfd = getListeningSocket(port);
  if (std::holds_alternative<Error>(listeningfd)) {
    return std::get<Error>(listeningfd);
  }

  for (auto& [fd, event, revent] : pollfds) {
    fd = -1;
    event = POLLIN;
    revent = 0;
  }

  pollfds[0].fd = std::get<int>(listeningfd);
  return std::nullopt;
}

std::variant<int, Error> Server::getListeningSocket(const Port port) {
  std::optional<Error> ret = std::nullopt;
  int socketfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (socketfd == -1) {
    return Error("ERROR: socket - " + std::string(strerror(errno)));
  }

  if (ret = acceptBothIPv4AndIPv6(socketfd); ret.has_value()) {
    close(socketfd);
    return ret.value();
  }

  if (ret = bindSocket(socketfd, port); ret.has_value()) {
    close(socketfd);
    return ret.value();
  }

  if (listen(socketfd, kListenBacklog) == -1) {
    close(socketfd);
    return Error("ERROR: listen - " + std::string(strerror(errno)));
  }

  return socketfd;
}

std::optional<Error> Server::acceptBothIPv4AndIPv6(const int socketfd) {
  constexpr int opt = 0;
  if (setsockopt(socketfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) ==
      -1) {
    return Error("ERROR: setsockopt - " + std::string(strerror(errno)));
  }
  return std::nullopt;
}

std::optional<Error> Server::bindSocket(const int socketfd, const Port port) {
  sockaddr_in6 address = {};
  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons(port.getPort().value_or(0));

  if (bind(socketfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
      -1) {
    return Error("ERROR: bind - " + std::string(strerror(errno)));
  }

  return std::nullopt;
}

int Server::run() {

}
