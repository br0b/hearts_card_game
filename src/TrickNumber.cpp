#include "TrickNumber.h"
#include "MaybeError.h"
#include "Utilities.h"

TrickNumber::TrickNumber() : number(1) {}

MaybeError TrickNumber::Parse(std::string str) {
  if (std::optional<int> num = Utilities::ParseNumber(str, 1, 13); num.has_value()) {
    number = num.value();
    return std::nullopt;
  } else {
    return Error::InvalidArgs("TrickNumber::Parse");
  }
}

MaybeError TrickNumber::Set(int number_) {
  if (number_ < lowerBound || number_ > upperBound) {
    return Error::InvalidArgs("TrickNumber::Set");
  }

  number = number_;
  return std::nullopt;
}

int TrickNumber::Get() const {
  return number;
}

std::ostream& operator<<(std::ostream &os, const TrickNumber &x) {
  os << x.number;
  return os;
}

bool operator==(const TrickNumber &l, const TrickNumber &r) {
  return l.number == r.number;
}

