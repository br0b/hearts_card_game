#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <memory>

class Error {
 public:
  Error(std::string funName, std::string message);

  [[nodiscard]] static std::unique_ptr<Error> FromErrno(std::string funName_);
  [[nodiscard]] static std::unique_ptr<Error> InvalidArgs(std::string funName_);
  [[nodiscard]] static std::unique_ptr<Error> OutOfRange(std::string funName_);

  // Argument funName is the name of the function that set the errno.
  void SetFunName(std::string funName_);

  [[nodiscard]] std::string GetMessage() const;

 private:
  std::string funName;
  std::string message;
};

#endif  // ERROR_H
