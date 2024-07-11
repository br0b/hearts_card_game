//
// Created by robert-grigoryan on 5/31/24.
//

#include <utility>

#include "../include/Error.h"

Error::Error() = default;

Error::~Error() = default;

Error::Error(std::string _message) : message(std::move(_message)) {}

std::string Error::getMessage() const {
  return message;
}

bool Error::isCritical() const {
  return false;
}