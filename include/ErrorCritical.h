//
// Created by robert-grigoryan on 6/5/24.
//
#ifndef ERRORSYSTEM_H
#define ERRORSYSTEM_H

#include "Error.h"

class ErrorCritical final : public Error {
 public:
  ErrorCritical();
  ErrorCritical(const std::string &funName, const std::string &message_);
  explicit ErrorCritical(const std::string &_message);
  static std::unique_ptr<ErrorCritical> FromErrno(const std::string &funName);

  [[nodiscard]] bool IsCritical() const override;
};



#endif //ERRORSYSTEM_H
