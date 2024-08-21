#include <sstream>

#include "Hand.h"
#include "MessagePlayTrick.h"

void MessagePlayTrick::SetCard(Card card_) {
  card = card_;
}

Card MessagePlayTrick::GetCard() const {
  return card;
}

std::string MessagePlayTrick::Str() const {
  std::ostringstream oss;
  oss << "!" << card;
  return oss.str();
}

std::optional<std::string> MessagePlayTrick::UserStr() const {
  return std::nullopt;
}

MaybeError MessagePlayTrick::SetAfterMatch(std::smatch match) {
  Hand hand;
  hand.Parse(match[1]);
  if (hand.Get().size() != 1) {
    return std::make_unique<Error>("MessagePlayTrick::SetAfterMatch",
                                   "Wrong format");
  }
  card = hand.Get().at(0);
  return std::nullopt;
}

std::string MessagePlayTrick::GetPattern() const {
  return "^!((?:10|[2-9]|[JQKA])[CDHS])$";
}

