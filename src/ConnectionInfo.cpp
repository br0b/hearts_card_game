//
// Created by robert-grigoryan on 6/1/24.
//
#include "ConnectionInfo.h"

ConnectionInfo::ConnectionInfo(const int fd, const sockaddr address,
                               const socklen_t len)
    : fd(fd), address(address), len(len) {}

int ConnectionInfo::getFd() const {
  return fd;
}

sockaddr ConnectionInfo::getAddress() const {
  return address;
}

socklen_t ConnectionInfo::getLen() const {
  return len;
}
