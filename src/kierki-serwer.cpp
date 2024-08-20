#include <memory>
#include <variant>

#include "GameConfig.h"
#include "MaybeError.h"
#include "Logger.h"
#include "Server.h"
#include "ServerConfig.h"

int main(int argc, char *argv[]) {
  const std::string separator = "\r\n";

  MaybeError error = std::nullopt;
  std::unique_ptr<ServerConfig> serverConfig;
  GameConfig gameConfig;
  Server server;
  // server.EnableDebug();

  auto tmp = ServerConfig::FromMainArgs(argc, argv);
  if (std::holds_alternative<MaybeError>(tmp)) {
    Logger::Log(std::get<MaybeError>(tmp).value()->GetMessage());
    return 1;
  }
  serverConfig = std::move(std::get<std::unique_ptr<ServerConfig>>(tmp));

  if (error = gameConfig.Set(serverConfig->GetConfigFile());
      error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  server.Configure(gameConfig.Get(), serverConfig->GetMaxTimeout());

  if (error = server.Listen(serverConfig->GetPort()); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  if (error = server.Run(); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  return 0;
}
