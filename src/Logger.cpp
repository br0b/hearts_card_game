//
// Created by robert-grigoryan on 5/30/24.
//
#include <chrono>

#include "Logger.h"

void Logger::log(const std::string& message) {
  std::cerr << message << std::endl;
}

void Logger::report(const std::string& message) {
  std::cout << message << std::endl;
}
