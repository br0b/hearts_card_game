//
// Created by robert-grigoryan on 6/5/24.
//
#ifndef ERRORSYSTEM_H
#define ERRORSYSTEM_H

#include "Error.h"

class ErrorCritical final : public Error {
 public:
  [[nodiscard]] bool isCritical() const override;
  ErrorCritical();
  explicit ErrorCritical(std::string _message);
};



#endif //ERRORSYSTEM_H
