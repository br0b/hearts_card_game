#include <memory>
#include <sstream>
#include <string.h>

#include "Error.h"

Error::Error(std::string funName, std::string message)
    : funName(std::move(funName)), message(std::move(message)) {}

void Error::SetFunName(std::string funName_) {
  funName = funName_;
}

std::unique_ptr<Error> Error::FromErrno(std::string funName_) {
  return std::make_unique<Error>(std::move(funName_), strerror(errno));
}

std::unique_ptr<Error> Error::InvalidArgs(std::string funName_) {
  return std::make_unique<Error>(std::move(funName_), "Invalid arguments.");
}

std::unique_ptr<Error> Error::InvalidArg(std::string funName_,
                                        std::string argName) {
  return std::make_unique<Error>(
      std::move(funName_),
      "Argument \"" + std::move(argName) + "\" is invalid.");
}

std::unique_ptr<Error> Error::ArgOmitted(std::string funName_,
                                         std::string argName) {
  return std::make_unique<Error>(
      std::move(funName_),
      "Argument \"" + std::move(argName) + "\" was ommited.");
}

std::unique_ptr<Error> Error::OutOfRange(std::string funName_) {
  return std::make_unique<Error>(std::move(funName_), "Out of range.");
}

std::string Error::GetMessage() const {
  std::ostringstream oss;
  oss << "ERROR: " << funName << " - " << message;
  return oss.str();
}

