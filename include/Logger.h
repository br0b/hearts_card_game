//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
#include <string>

class Logger {
public:
  static void log(const std::string& message) {
    std::cerr << message << std::endl;
  }

  static void repport(const std::string& message) {
    std::cerr << message << std::endl;
  }
};

#endif  // LOGGER_H
