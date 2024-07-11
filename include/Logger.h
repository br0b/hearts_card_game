//
// Created by robert-grigoryan on 5/30/24.
//
#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
#include <string>

class Logger {
 public:
  static void log(const std::string& message);
  static void report(const std::string& message);
};

#endif  // LOGGER_H
