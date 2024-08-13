#include <regex>
#include "MaybeError.h"
#include "TrickNumber.h"

#include "MessageTaken.h"

void MessageTaken::SetTrickNumber(TrickNumber trickNumber_) {
  trickNumber = trickNumber_;
}

void MessageTaken::SetCards(Hand cards_) {
  cards = cards_;
}

void MessageTaken::SetTaker(Seat taker_) {
  taker = taker_;
}

TrickNumber MessageTaken::GetTrickNumber() const {
  return trickNumber;
}

const Hand& MessageTaken::GetCards() const {
  return cards;
}

Seat MessageTaken::GetTaker() const {
  return taker;
}

std::ostream& operator<<(std::ostream &os, const MessageTaken &msg) {
  os << "TAKEN" << msg.trickNumber << msg.cards << msg.taker;
  return os;
}

MaybeError MessageTaken::SetAfterMatch(std::smatch match) {
  MaybeError error;

  if (error = trickNumber.Parse(match[1]), error.has_value()) {
    return error;
  }

  cards.Parse(match[2]);

  if (error = taker.Parse(match[3]); error.has_value()) {
    return error;
  }

  return std::nullopt;
}

std::string GetPattern() {
  return "^TAKEN([1-9]|1[0-3])((?:(?:10|[2-9]|[JQKA])[CDHS]){4})([NESW])$";
}

