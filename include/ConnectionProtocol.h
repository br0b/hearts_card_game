//
// Created by robert-grigoryan on 7/11/24.
//
#ifndef CONNECTIONPROTOCOL_H
#define CONNECTIONPROTOCOL_H
#include <ostream>

enum class ConnectionProtocol { kIPv4, kIPv6 };

std::ostream &operator<<(std::ostream &os, const ConnectionProtocol &protocol);

#endif //CONNECTIONPROTOCOL_H
