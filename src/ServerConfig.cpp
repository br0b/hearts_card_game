#include <unistd.h>
#include <memory>
#include <string>

#include "MaybeError.h"
#include "ServerConfig.h"
#include "Utilities.h"

ServerConfig::ServerConfig(std::string configFile)
    : configFile(std::move(configFile)) {}

std::variant<std::unique_ptr<ServerConfig>, MaybeError>
ServerConfig::FromMainArgs(int argc, char *argv[]) {
  std::optional<int> maxTimeout;
  std::optional<std::string> configFile;
  std::optional<uint16_t> port;
  int opt;

  while ((opt = getopt(argc, argv, "p:f:t:")) != -1) {
    switch (opt) {
      case 'p':
        port = Utilities::ParseNumber<in_port_t>(optarg, 0, UINT16_MAX);
        if (!port.has_value()) {
          return Error::InvalidArg("ServerConfig::FromMainArgs", "port");
        }
        break;
      case 'f':
        configFile = optarg;
        break;
      case 't':
        maxTimeout = Utilities::ParseNumber<int>(optarg, 0, INT_MAX);
        if (!maxTimeout.has_value()) {
          return Error::InvalidArg("ServerConfig::FromMainArgs", "timeout");
        }
        break;
      default:
        return Error::InvalidArg("ServerConfig::FromMainArgs", optarg);
    }
  }

  if (!configFile.has_value()) {
    return Error::ArgOmitted("ServerConfig::FromMainArgs", "file");
  }

  auto config = std::make_unique<ServerConfig>(*configFile);
  if (port.has_value()) {
    config->SetPort(port.value());
  }
  if (maxTimeout.has_value()) {
    config->SetMaxTimeout(std::chrono::seconds(maxTimeout.value()));
  }
  return config;
}

void ServerConfig::SetPort(in_port_t port_) {
  port = port_;
}

void ServerConfig::SetMaxTimeout(std::chrono::seconds maxTimeout_) {
  maxTimeout = maxTimeout_;
}

std::optional<in_port_t> ServerConfig::GetPort() const {
  return port;
}

const std::string &ServerConfig::GetConfigFile() const {
  return configFile;
}

std::optional<std::chrono::seconds> ServerConfig::GetMaxTimeout() const {
  return maxTimeout;
}

