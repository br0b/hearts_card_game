#include "TrickNumber.h"
#include "MaybeError.h"
#include "Utilities.h"

TrickNumber::TrickNumber() : number(1) {}

MaybeError TrickNumber::Parse(std::string str) {
  if (std::optional<int> num = Utilities::ParseInt(str, 1, 13); num.has_value()) {
    number = num.value();
    return std::nullopt;
  } else {
    return Error::InvalidArgs("TrickNumber::Parse");
  }
}

std::ostream& operator<<(std::ostream &os, const TrickNumber &x) {
  os << x.number;
  return os;
}

