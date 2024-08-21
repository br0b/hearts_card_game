#include <regex>
#include <sstream>

#include "MaybeError.h"
#include "MessageTaken.h"
#include "TrickNumber.h"
#include "Utilities.h"


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

std::string MessageTaken::Str() const {
  std::ostringstream oss;
  oss << "TAKEN" << trickNumber << cards << taker;
  return oss.str();
}

std::optional<std::string> MessageTaken::UserStr() const {
  std::ostringstream oss;
  oss << "A trick " << trickNumber << " is taken by " << taker
      << ", cards ";
  Utilities::StrList(oss, cards.Get().begin(), cards.Get().end());
  oss << '.';
  return oss.str();
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

std::string MessageTaken::GetPattern() const {
  return "^TAKEN([1-9]|1[0-3])((?:(?:10|[2-9]|[JQKA])[CDHS]){4})([NESW])$";
}

