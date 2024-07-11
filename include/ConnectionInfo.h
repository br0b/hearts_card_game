//
// Created by robert-grigoryan on 6/1/24.
//
#ifndef CONNECTIONINFO_H
#define CONNECTIONINFO_H

#include <netinet/in.h>

class ConnectionInfo {
 public:
  ConnectionInfo(int fd, sockaddr address, socklen_t len);
  [[nodiscard]] int getFd() const;
  [[nodiscard]] sockaddr getAddress() const;
  [[nodiscard]] socklen_t getLen() const;

 private:
  const int fd;
  const sockaddr address;
  const socklen_t len;
};

#endif //CONNECTIONINFO_H
