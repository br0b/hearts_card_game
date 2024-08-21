#include <sstream>

#include "MaybeError.h"
#include "MessageTrick.h"
#include "TrickNumber.h"
#include "Utilities.h"

void MessageTrick::SetTrickNumber(TrickNumber trickNumber_) {
  trickNumber = trickNumber_;
}

void MessageTrick::SetCards(Hand cards_) {
  cards = cards_;
}

TrickNumber MessageTrick::GetTrickNumber() const {
  return trickNumber;
}

const Hand &MessageTrick::GetCards() const {
  return cards;
}

std::string MessageTrick::Str() const {
  std::ostringstream oss;
  oss << "TRICK" << trickNumber << cards;
  return oss.str();
}

std::optional<std::string> MessageTrick::UserStr() const {
  std::ostringstream oss;
  oss << "Trick: (" << trickNumber << ") ";
  Utilities::StrList(oss, cards.Get().begin(), cards.Get().end());
  return oss.str();
}

MaybeError MessageTrick::SetAfterMatch(std::smatch match) {
  cards.Parse(match[2]);
  return trickNumber.Parse(match[1]);
}

std::string MessageTrick::GetPattern() const {
  return "^TRICK([1-9]|1[0-3])((?:(?:10|[2-9]|[JQKA])[CDHS]){0,3})$";
}

