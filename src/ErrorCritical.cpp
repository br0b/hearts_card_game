#include "ErrorCritical.h"

ErrorCritical::ErrorCritical() : Error() {}

ErrorCritical::ErrorCritical(const std::string &message_) : Error(message_) {}

ErrorCritical::ErrorCritical(const std::string &funName,
                             const std::string &message_)
  : Error(funName, message_) {}

std::unique_ptr<ErrorCritical> ErrorCritical::FromErrno(
    const std::string &funName) {
  return std::make_unique<ErrorCritical>(
      *Error::CreateErrMsgFromErrno(funName));
}

bool ErrorCritical::IsCritical() const {
  return true;
}

