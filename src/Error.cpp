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
  return std::make_unique<Error>(std::move(funName_), "Invalid argument.");
}

std::unique_ptr<Error> Error::OutOfRange(std::string funName_) {
  return std::make_unique<Error>(std::move(funName_), "Out of range.");
}

std::string Error::GetMessage() const {
  std::ostringstream oss;
  oss << "ERROR: " << funName << " - " << message;
  return oss.str();
}

