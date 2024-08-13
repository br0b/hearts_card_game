#include "Seat.h"
#include "MaybeError.h"

MaybeError Seat::Parse(std::string str) {
  if (str.size() != 1) {
    return Error::InvalidArgs("Seat::Parse");
  }

  for (int i = 0; i < 4; i++) {
    if (str[0] == chars[i]) {
      value = static_cast<Value>(i);
      return std::nullopt;
    }
  }

  return Error::InvalidArgs("Seat::Parse");
}

void Seat::Set(Seat::Value value_) {
  value = value_;
}

void Seat::CycleClockwise() {
  value = static_cast<Value>((static_cast<int>(value) + 1) % 4);
}

Seat::Value Seat::Get() const {
  return value;
}

size_t Seat::GetIndex() const {
  return static_cast<size_t>(value);
}

std::ostream& operator<<(std::ostream& os, const Seat& seat) {
  os << seat.chars[static_cast<int>(seat.value)];
  return os;
}

