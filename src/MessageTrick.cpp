#include "MessageTrick.h"
#include "MaybeError.h"
#include "TrickNumber.h"

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

std::ostream &operator<<(std::ostream &os, const MessageTrick &msg) {
  os << "TRICK" << msg.trickNumber << msg.cards;
  return os;
}

MaybeError MessageTrick::SetAfterMatch(std::smatch match) {
  cards.Parse(match[2]);
  return trickNumber.Parse(match[1]);
}

std::string MessageTrick::GetPattern() {
  return "^TRICK([1-9]|1[0-3])((?:(?:10|[2-9]|[JQKA])[CDHS]){0,3})$";
}

