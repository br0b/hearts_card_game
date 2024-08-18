#include <memory>
#include <variant>

#include "Client.h"
#include "ClientConfig.h"
#include "Logger.h"
#include "MaybeError.h"

int main(int argc, char *argv[]) {
  MaybeError error = std::nullopt;
  std::unique_ptr<ClientConfig> config;

  auto tmp = ClientConfig::FromMainArgs(argc, argv);
  if (std::holds_alternative<MaybeError>(tmp)) {
    Logger::Log(std::get<MaybeError>(tmp).value()->GetMessage());
    return 1;
  }
  config = std::move(std::get<std::unique_ptr<ClientConfig>>(tmp));

  Client client(config->GetSeat());

  if (error = client.Connect(config->GetHost(), config->GetPort(),
                             config->GetProtocol());
      error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  if (error = client.Run(config->IsAutomatic()); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  return 0;
}
