#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory>

#include "Client.h"
#include "ConnectionProtocol.h"
#include "MaybeError.h"
#include "MessageBuffer.h"
#include "Utilities.h"

Client::Client(Seat seat, size_t bufferLen, std::string separator)
    : seat(seat), buffer(bufferLen), server(buffer, separator) {}

MaybeError Client::Connect(
    std::string host,
    in_port_t port,
    std::optional<ConnectionProtocol> protocol) {
  struct sockaddr_storage serverAddr;
  Utilities::Socket s;
  MaybeError error = std::nullopt;
  int addrFam = GetAddressFamily(protocol);

  if (error = Utilities::CreateAddress(host, port, addrFam, serverAddr);
      error.has_value()) {
    return error;
  }

  if (error = Utilities::GetBoundSocket(addrFam, s); error.has_value()) {
    return error;
  }

  if (error = Utilities::ConnectSocket(s.fd, serverAddr);
      error.has_value()) {
    return error;
  }

  pollfds.emplace_back(s.fd, POLLIN, 0);
  // TODO: Set messageBuffers.
  return std::nullopt;
}

MaybeError Client::Run(bool isAutomatic) {
  MaybeError error = std::nullopt;

  if (!isAutomatic) {
    user = std::make_unique<MessageBuffer>(buffer, "\n");
    user->SetPipe(STDIN_FILENO);
    pollfds.emplace_back(STDIN_FILENO, POLLIN, 0);
  }

  // TODO: Implement server request handler.

  return std::nullopt;
}

int Client::GetAddressFamily(std::optional<ConnectionProtocol> protocol) {
  if (!protocol.has_value()) {
    return AF_UNSPEC;
  } else if (protocol == ConnectionProtocol::kIPv4) {
    return AF_INET;
  } else {
    return AF_INET6;
  }
}

