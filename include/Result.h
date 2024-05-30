//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef EXPECTED_H
#define EXPECTED_H

#include <variant>

template<typename T, typename E>
class Result {
   private:
    std::variant<T, E> value;

    explicit Result(const T &_value) : value(_value) {}
    explicit Result(const E &_error) : value(_error) {}

   public:
    static Result Success(const T &_value) { return Result(_value); }
    static Result Error(const E &_error) { return Result(_error); }

    [[nodiscard]] bool isSuccess() const {
        return std::holds_alternative<T>(value);
    }
    [[nodiscard]] bool isError() const {
        return std::holds_alternative<E>(value);
    }

    [[nodiscard]] T getValue() const { return std::get<T>(value); }
    [[nodiscard]] E getError() const { return std::get<E>(value); }
};

#endif //EXPECTED_H
