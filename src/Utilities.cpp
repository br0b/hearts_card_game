#include <arpa/inet.h>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>

#include "MaybeError.h"
#include "Utilities.h"

MaybeError Utilities::CreateAddress(std::string host, in_port_t port,
                                    int &addrFam,
                                    struct sockaddr_storage &address) {
  addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = addrFam;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  addrinfo *addressResult;
  int errcode = getaddrinfo(host.c_str(), std::to_string(port).c_str(),
                            &hints, &addressResult);
  if (errcode != 0) {
    return std::make_unique<Error>("getaddrinfo", gai_strerror(errcode));
  }

  addrFam = addressResult->ai_family;
  memcpy(&address, addressResult->ai_addr, addressResult->ai_addrlen);
  freeaddrinfo(addressResult);

  return std::nullopt;
}

MaybeError Utilities::GetBoundSocket(int ai_family, Socket &s) {
  struct sockaddr_storage addr_storage;
  in_port_t port = htons(s.port.value_or(0));
  int fd;

  if (fd = socket(ai_family, SOCK_STREAM, 0); fd < 0) {
    return Error::FromErrno("socket");
  }

  memset(&addr_storage, 0, sizeof(addr_storage));

  if (ai_family == AF_INET) {
    struct sockaddr_in *addr = (struct sockaddr_in *)&addr_storage;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = port;
  } else {
    struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&addr_storage;
    addr->sin6_family = AF_INET6;
    addr->sin6_addr = in6addr_any;
    addr->sin6_port = port;
  }

  if (bind(fd, (struct sockaddr *)&addr_storage,
           sizeof(addr_storage)) < 0) {
    return Error::FromErrno("bind");
  }

  if (MaybeError error = GetAddressFromFd(fd, addr_storage);
      error.has_value()) {
    return error;
  }

  if (MaybeError error = GetPortFromAddress(addr_storage, port);
      error.has_value()) {
    return error;
  }

  s.fd = fd;
  s.port = port;
  return std::nullopt;
}

MaybeError Utilities::ConnectSocket(
    int fd,
    const struct sockaddr_storage &address) {
  if (connect(fd, (sockaddr *)&address, sizeof(address)) != 0) {
    return Error::FromErrno("connect");
  }

  return std::nullopt;
}

MaybeError Utilities::GetStringFromAddress(
    const struct sockaddr_storage &address,
    std::string &addressStr) {
  std::ostringstream oss;
  std::string ip;
  uint16_t port;
  MaybeError error;

  error = GetIpFromAddress(address, ip);
  if (error.has_value()) {
    return error;
  }

  error = GetPortFromAddress(address, port);
  if (error.has_value()) {
    return error;
  }

  oss << ip << ":" << port;
  addressStr = oss.str();
  return std::nullopt;
}

std::string Utilities::GetTimeStr() {
  const auto now = std::chrono::system_clock::now();
  const auto now_time_t = std::chrono::system_clock::to_time_t(now);
  const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()) % 1000;

  // Convert to local time
  std::tm now_tm{};
  localtime_r(&now_time_t, &now_tm);

  // Create a string stream to format the time
  std::ostringstream oss;
  oss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%S")
      << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
  return oss.str();
}

MaybeError Utilities::GetAddressFromFd(
    int socketFd,
    struct sockaddr_storage &address) {
  socklen_t len = sizeof(address);
  memset(&address, 0, len);
  if (getsockname(socketFd, (struct sockaddr *)&address, &len) < 0) {
    return Error::FromErrno("getsockname");
  }

  // Check if the result was truncated.
  if (len > sizeof(address)) {
    return std::make_unique<Error>("Utilities::GetAddressFromFd",
                                   "Provided buffer was too small.");
  }

  return std::nullopt;
}

MaybeError Utilities::GetAddressPair(int fd, std::string &local,
                                     std::string &remote) {
  if (MaybeError error = GetAddressStrFromFd(fd, local); error.has_value()) {
    return error;
  }

  return GetRemoteFromFd(fd, remote);
}

MaybeError Utilities::GetAddressStrFromFd(
    int socketFd,
    std::string &address) {
  struct sockaddr_storage addr;
  MaybeError error;

  if (error = GetAddressFromFd(socketFd, addr); error.has_value()) {
    return error;
  }

  if (error = GetStringFromAddress(addr, address); error.has_value()) {
    return error;
  }
  
  return std::nullopt;
}

MaybeError Utilities::GetRemoteFromFd(int fd, std::string &address) {
  sockaddr_storage addr;
  socklen_t len = sizeof(addr);

  memset(&addr, 0, len);
  if (getpeername(fd, (sockaddr *)&addr, &len) < 0) {
    return Error::FromErrno("getpeername");
  }

  if (sizeof(addr) < len) {
    return std::make_unique<Error>("Utilities::GetRemoteFromFd",
                                   "Provided buffer was too small.");
  }

  return GetStringFromAddress(addr, address);
}

MaybeError Utilities::GetPortFromFd(int fd, in_port_t &port) {
  sockaddr_storage addr;

  if (MaybeError error = GetAddressFromFd(fd, addr); error.has_value()) {
    return error;
  }

  return GetPortFromAddress(addr, port);
}

MaybeError Utilities::GetIpFromAddress(
    const struct sockaddr_storage &address,
    std::string &ipStr) {
  std::array<char, INET6_ADDRSTRLEN> cStr;
  const void *addr = nullptr;

  switch (address.ss_family) {
    case AF_INET:
      addr = &reinterpret_cast<const struct sockaddr_in&>(address).sin_addr;
      break;
    case AF_INET6:
      addr = &reinterpret_cast<const struct sockaddr_in6&>(address).sin6_addr;
      break;
    default:
      return std::make_unique<Error>("GetIpFromAddress",
                                     "Unsupported protocol.");
  }

  if (inet_ntop(address.ss_family, addr, &cStr[0], cStr.size()) == NULL) {
    return Error::FromErrno("inet_ntop");
  }

  ipStr = &cStr[0];
  return std::nullopt;
}

MaybeError Utilities::GetPortFromAddress(
    const struct sockaddr_storage &address,
    uint16_t &port) {
  switch (address.ss_family) {
    case AF_INET:
      port = reinterpret_cast<const struct sockaddr_in&>(address).sin_port;
      break;
    case AF_INET6:
      port = reinterpret_cast<const struct sockaddr_in6&>(address).sin6_port;
      break;
    default:
      return std::make_unique<Error>("GetPortFromAddress",
                                     "Unsupported protocol.");
  }

  // Convert to host byte order.
  port = ntohs(port);

  return std::nullopt;
}

