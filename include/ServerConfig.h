#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <chrono>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <variant>

#include "MaybeError.h"

class ServerConfig {
 public:
  ServerConfig(std::string configFile);

  [[nodiscard]] static std::variant<std::unique_ptr<ServerConfig>, MaybeError>
    FromMainArgs(int argc, char *argv[]);

  void SetPort(in_port_t port_);
  void SetMaxTimeout(std::chrono::seconds maxTimeout_);

  [[nodiscard]] std::optional<in_port_t> GetPort() const;
  [[nodiscard]] const std::string &GetConfigFile() const;
  [[nodiscard]] std::optional<std::chrono::seconds> GetMaxTimeout() const;

 private:
  std::optional<in_port_t> port;
  std::string configFile;
  std::optional<std::chrono::seconds> maxTimeout;
};

#endif  // SERVER_CONFIG_H

