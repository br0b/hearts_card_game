#include <memory>

#include "Client.h"
#include "ClientConfig.h"
#include "Logger.h"
#include "MaybeError.h"

MaybeError GetClientConfig(std::unique_ptr<ClientConfig> &config) {
  Seat seat;
  if (MaybeError error = seat.Set('N'); error.has_value()) {
    return error;
  }
  config = std::make_unique<ClientConfig>("localhost", 42000,
                                          ConnectionProtocol::kIPv4, seat,
                                          true);
  return std::nullopt;
}

int main() {
  MaybeError error = std::nullopt;
  const size_t kBufLen = 4096;
  const std::string kSeparator = "\r\n";
  std::unique_ptr<ClientConfig> config; 

  if (error = GetClientConfig(config); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  Client client(config->GetSeat(), kBufLen, kSeparator);

  if (error = client.Connect(config->GetHost(), config->GetPort(),
                             config->GetProtocol());
      error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  if (error = client.Run(config->GetIsAutomatic()); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  return 0;
}
