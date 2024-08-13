#ifndef ERROR_H
#define ERROR_H

#include <memory>
#include <string>

class Error {
 public:
  Error();
  Error(std::string message);
  Error(std::string funName, std::string message);

  virtual ~Error();

  // Argument funName is the name of the function that set the errno.
  static std::unique_ptr<Error> FromErrno(std::string funName);

  static std::unique_ptr<Error> InvalidArgs(std::string funName);

  [[nodiscard]] const std::string& GetMessage() const;
  [[nodiscard]] virtual bool IsCritical() const;

 protected:
  std::string message;
 
 private:
  static std::unique_ptr<std::string> CreateErrMsgFromErrno(
      std::string funName);
};

#endif  // ERROR_H
