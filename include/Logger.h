#ifndef LOGGER_H
#define LOGGER_H
#include <string>

class Logger {
 public:
  // Print to stderr.
  static void Log(const std::string &message);

  // Print to stdout.
  static void Report(const std::string &message);
};

#endif  // LOGGER_H
