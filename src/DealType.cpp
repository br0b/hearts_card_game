#include "DealType.h"
#include "MaybeError.h"
#include "Utilities.h"

MaybeError DealType::Parse(std::string str) {
  std::optional<int> type = Utilities::ParseNumber(str, 1, 7);

  if (!type.has_value()) {
    return Error::InvalidArgs("DealType::Parse");
  }

  value = static_cast<Value>(type.value() - 1);
  return std::nullopt;
}

DealType::Value DealType::Get() const {
  return value;
}

std::ostream& operator<<(std::ostream &os, const DealType &dt) {
  os << static_cast<int>(dt.value) + 1;
  return os;
}
