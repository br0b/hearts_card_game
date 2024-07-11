//
// Created by robert-grigoryan on 6/7/24.
//
#include <arpa/inet.h>

#include <chrono>
#include <cstring>
#include <iostream>

#include "ErrorCritical.h"
#include "Utilities.h"

std::string Utilities::getTimeStr() {
  const auto now = std::chrono::system_clock::now();
  const auto now_time_t = std::chrono::system_clock::to_time_t(now);
  const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;

  // Convert to local time
  std::tm now_tm{};
#ifdef _WIN32
  localtime_s(&now_tm, &now_time_t);
#else
  localtime_r(&now_time_t, &now_tm);
#endif

  // Create a string stream to format the time
  std::ostringstream oss;
  oss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

  return oss.str();
}

std::variant<std::string, Error> Utilities::getStringFromAddress(
    sockaddr_in6 address) {
  char ip_str[INET6_ADDRSTRLEN];  // Buffer to hold the IPv6 string

  // Convert IPv6 address to string
  if (inet_ntop(AF_INET6, &address.sin6_addr, ip_str, sizeof(ip_str)) ==
      nullptr) {
    return ErrorCritical("ERROR: inet_ntop - " + std::string(strerror(errno)));
  }

  // Convert port from network byte order to host byte order
  uint16_t port = ntohs(address.sin6_port);

  // Format the string as <ipv6>:<port>
  std::ostringstream oss;
  oss << ip_str << ":" << port;

  return oss.str();
}

std::variant<std::string, Error> Utilities::getAddressStrFromFd(int socketfd) {
  auto address = getAddressFromFd(socketfd);
  if (std::holds_alternative<Error>(address)) {
    return std::get<Error>(address);
  }
  return getStringFromAddress(std::get<sockaddr_in6>(address));
}

std::variant<sockaddr_in6, Error> Utilities::getAddressFromFd(int socketfd) {
  if (socketfd == -1) {
    return ErrorCritical("Socket not yet set.");
  }

  sockaddr_in6 address = {};
  socklen_t len = sizeof(address);
  if (getsockname(socketfd, reinterpret_cast<sockaddr*>(&address), &len) < 0) {
    return ErrorCritical("ERROR: getsockname - " +
                         std::string(strerror(errno)));
  }
  if (sizeof(address) != len) {
    return ErrorCritical("ERROR: getsockname - invalid address length.");
  }

  return address;
}
