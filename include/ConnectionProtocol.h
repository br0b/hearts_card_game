//
// Created by robert-grigoryan on 7/11/24.
//
#ifndef CONNECTIONPROTOCOL_H
#define CONNECTIONPROTOCOL_H
#include <ostream>

class ConnectionProtocol {
public:
  enum class Code { kIPv4, kIPv6 };

  [[nodiscard]] Code getCode() const;
  friend std::ostream& operator<<(std::ostream& os,
                                  const ConnectionProtocol& protocol);

  explicit ConnectionProtocol(Code code);

private:
  Code code;
};

#endif //CONNECTIONPROTOCOL_H
