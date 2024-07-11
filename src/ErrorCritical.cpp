//
// Created by robert-grigoryan on 6/5/24.
//
#include <utility>

#include "ErrorCritical.h"

bool ErrorCritical::isCritical() const { return true; }

ErrorCritical::ErrorCritical() = default;

ErrorCritical::ErrorCritical(std::string _message)
    : Error(std::move(_message)) {}
