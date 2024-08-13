#include <memory>

#include "GameConfig.h"
#include "MaybeError.h"
#include "Logger.h"
#include "Server.h"

MaybeError GetServerConfig(GameConfig &config) {
  return config.Set({
    "1N",
    "7S10SAC8H3C5HQHQD7H10D4C7DKH",
    "9D3S9C2SQS5C2C6D5SKC6H9H7C",
    "KSJHQCAD10HAS4H3H6S5D8S8DKD",
    "8CJD3D4S9SJC10C6C4DJSAH2H2D",
  });
}

int main() {
  const std::string separator = "\r\n";
  const size_t kBufferLen = 4096;
  const time_t kTimeout(5000);
  in_port_t port = 42000;
  const int kMaxTcpQueueLen = 5;

  MaybeError error = std::nullopt;
  auto config = std::make_unique<GameConfig>();
  auto server = std::make_unique<Server>(separator, kBufferLen, kMaxTcpQueueLen);

  if (error = GetServerConfig(*config); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  server->Configure(std::move(config));

  if (error = server->Listen(port, kTimeout); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  return 0;
}
