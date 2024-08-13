#include "MessageWrong.h"
#include "TrickNumber.h"

void MessageWrong::SetTrickNumber(TrickNumber trickNumber_) {
  trickNumber = trickNumber_;
}

[[nodiscard]] TrickNumber MessageWrong::GetTrickNumber() const {
  return trickNumber;
}

std::ostream &operator<<(std::ostream &os, const MessageWrong &msg) {
  os << "WRONG" << msg.trickNumber;
  return os;
}

MaybeError MessageWrong::SetAfterMatch(std::smatch match) {
  return trickNumber.Parse(match[1]);
}

std::string MessageWrong::GetPattern() {
  return "^WRONG([1-9]|1[0-3])$";
}

