#include <memory>
#include <sstream>
#include <string.h>

#include "Error.h"

Error::Error() = default;

Error::Error(std::string message) : message(std::move(message)) {}

Error::Error(std::string funName, std::string message) {
  std::ostringstream oss;
  oss << "ERROR: " << std::move(funName) << " " << std::move(message);
  Error(oss.str());
}

Error::~Error() = default;

std::unique_ptr<Error> Error::FromErrno(std::string funName) {
  std::string msg(*CreateErrMsgFromErrno(std::move(funName)));
  return std::make_unique<Error>();
}

std::unique_ptr<Error> Error::InvalidArgs(std::string funName) {
  return std::make_unique<Error>(std::move(funName), "Invalid arguments.");
}

const std::string& Error::GetMessage() const {
  return message;
}

bool Error::IsCritical() const {
  return false;
}

std::unique_ptr<std::string> Error::CreateErrMsgFromErrno(
    std::string funName) {
  std::ostringstream oss;
  oss << "ERROR: " << std::move(funName) << " "
      << std::string(strerror(errno));
  return std::make_unique<std::string>(oss.str());
}

