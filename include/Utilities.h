//
// Created by robert-grigoryan on 6/7/24.
//
#ifndef UTILITIES_H
#define UTILITIES_H

#include <netinet/in.h>

#include <string>

#include "Error.h"

class Utilities {
public:
  static std::string getTimeStr();
  static std::variant<sockaddr_in6, Error> getAddressFromFd(int socketfd);
  static std::variant<std::string, Error> getStringFromAddress(
      sockaddr_in6 address);
  static std::variant<std::string, Error> getAddressStrFromFd(int socketfd);
};

#endif  // UTILITIES_H
