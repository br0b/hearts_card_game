#ifndef UTILITIES_H
#define UTILITIES_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <string>

#include "MaybeError.h"

class Utilities {
public:
  struct Socket {
    std::optional<int> fd;
    std::optional<in_port_t> port;
  };

  // If addrFam = AF_UNSPEC, then on return it contains
  // the address family of the server.
  [[nodiscard]] static MaybeError CreateAddress(
      std::string host,
      in_port_t port,
      int &addrFam,
      struct sockaddr_storage &address);

  // To bind to any port, pass s.port = 0.
  [[nodiscard]] static MaybeError GetBoundSocket(int ai_family, Socket &s);

  [[nodiscard]] static MaybeError ConnectSocket(
      int fd,
      const struct sockaddr_storage &address);

  [[nodiscard]] static MaybeError GetStringFromAddress(
      const struct sockaddr_storage &address,
      std::string &addressStr);

  static std::string GetTimeStr();

  [[nodiscard]] static MaybeError GetAddressFromFd(
      int socketFd,
      struct sockaddr_storage &address);

  [[nodiscard]] static MaybeError GetAddressPair(int fd, std::string &local,
                                                 std::string &remote);

  [[nodiscard]] static MaybeError GetAddressStrFromFd(int socketFd,
                                                      std::string &address);

  [[nodiscard]] static MaybeError GetPortFromFd(int fd, in_port_t &port);

  [[nodiscard]] static MaybeError SetNonBlocking(int fd);

  // Parse string to number of type T in range [l;r].
  // Return std::nullopt otherwise.
  template <typename T>
  [[nodiscard]] static std::optional<T> ParseNumber(std::string str, T l,
                                                    T r) {
    int ret = 0;

    try {
      ret = std::stoi(str);

      if (ret < l || ret > r) {
        return std::nullopt;
      }
    } catch (std::logic_error &e) {
      return std::nullopt;
    }

    return ret;
  }

  template <typename Iterator>
  static void StrList(std::ostringstream &oss, Iterator begin, Iterator end) {
    if (begin == end) {
      return;
    }
    
    auto it = begin;
    oss << *it;
    it++;
    for (; it != end; it++) {
      oss << ", " << *it;
    }
  }

private:
  [[nodiscard]] static MaybeError GetIpFromAddress(
      const struct sockaddr_storage &address,
      std::string &ip);
  [[nodiscard]] static MaybeError GetPortFromAddress(
      const struct sockaddr_storage &address,
      in_port_t &port);
  [[nodiscard]] static MaybeError GetRemoteFromFd(int fd, std::string &addressIPv6);
};

#endif  // UTILITIES_H
