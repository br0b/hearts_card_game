//
// Created by robert-grigoryan on 5/31/24.
//

#ifndef ERROR_H
#define ERROR_H
#include <string>

class Error {
 public:
  explicit Error(std::string _message);
  [[nodiscard]] std::string getMessage() const;

 private:
  std::string message;
};

#endif  // ERROR_H
