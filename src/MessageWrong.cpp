#include "MessageWrong.h"
#include <sstream>
#include "TrickNumber.h"

void MessageWrong::SetTrickNumber(TrickNumber trickNumber_) {
  trickNumber = trickNumber_;
}

[[nodiscard]] TrickNumber MessageWrong::GetTrickNumber() const {
  return trickNumber;
}

std::string MessageWrong::Str() const {
  std::ostringstream oss;
  oss << "WRONG" << trickNumber;
  return oss.str();
}

std::optional<std::string> MessageWrong::UserStr() const {
  std::ostringstream oss;
  oss << "Wrong message received in trick " << trickNumber << '.';
  return oss.str();
}

MaybeError MessageWrong::SetAfterMatch(std::smatch match) {
  return trickNumber.Parse(match[1]);
}

std::string MessageWrong::GetPattern() const {
  return "^WRONG([1-9]|1[0-3])$";
}

