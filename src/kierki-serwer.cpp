#include <memory>

#include "GameConfig.h"
#include "MaybeError.h"
#include "Logger.h"
#include "Server.h"

int main() {
  const std::string separator = "\r\n";
  const size_t kBufferLen = 4096;
  const time_t kTimeout(5000);
  in_port_t port = 42000;
  const int kMaxTcpQueueLen = 5;

  MaybeError error = std::nullopt;
  GameConfig config;
  Server server(separator, kBufferLen, kMaxTcpQueueLen);

  if (error = config.Set("config.txt"); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  server.Configure(config.Get());

  if (error = server.Listen(port, kTimeout); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  if (error = server.Run(); error.has_value()) {
    Logger::Log(error.value()->GetMessage());
    return 1;
  }

  return 0;
}
