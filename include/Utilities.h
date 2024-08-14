#ifndef UTILITIES_H
#define UTILITIES_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

#include "MaybeError.h"

class Utilities {
public:
  struct Socket {
    int fd;
    in_port_t port;
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

  // Parse string to integer in range [l;r]. Return std::nullopt otherwise.
  [[nodiscard]] static std::optional<int> ParseInt(std::string tr, int l,
                                                   int r);

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
