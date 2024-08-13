#ifndef RESULT_H
#define RESULT_H

#include <memory>
#include <variant>

#include "Error.h"

// Stores std::unique_ptr<T> or Error.
template <typename T>
class Result {
 public:
  [[nodiscard]] bool isError() const;

  [[nodiscard]] std::unique_ptr<T> getValue();

  [[nodiscard]] Error getError();

  static Result<T> Success(std::unique_ptr<T> value);

  static Result<T> Failure(const Error& error);

 private:
  std::variant<std::unique_ptr<T>, Error> value;

  explicit Result(std::unique_ptr<T> value) : value(std::move(value)) {}

  explicit Result(Error error) : value(error) {}
};

template<typename T>
bool Result<T>::isError() const {
  return std::holds_alternative<Error>(value);
}

template<typename T>
std::unique_ptr<T> Result<T>::getValue() {
  return std::move(std::get<std::unique_ptr<T>>(value));
}

template<typename T>
Error Result<T>::getError() {
  return std::move(std::get<Error>(value));
}

template<typename T>
Result<T> Result<T>::Success(std::unique_ptr<T> value) {
  return Result(std::move(value));
}

template<typename T>
Result<T> Result<T>::Failure(const Error& error) {
  return Result(error);
}

#endif //RESULT_H
