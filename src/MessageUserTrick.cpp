#include <sstream>

#include "Hand.h"
#include "MessageUserTrick.h"

void MessageUserTrick::SetCard(Card card_) {
  card = card_;
}

Card MessageUserTrick::GetCard() const {
  return card;
}

std::string MessageUserTrick::Str() const {
  std::ostringstream oss;
  oss << "!" << card;
  return oss.str();
}

std::optional<std::string> MessageUserTrick::UserStr() const {
  return std::nullopt;
}

MaybeError MessageUserTrick::SetAfterMatch(std::smatch match) {
  Hand hand;
  hand.Parse(match[1]);
  if (hand.Get().size() != 1) {
    return std::make_unique<Error>("MessagePlayTrick::SetAfterMatch",
                                   "Wrong format");
  }
  card = hand.Get().at(0);
  return std::nullopt;
}

std::string MessageUserTrick::GetPattern() const {
  return "^!((?:10|[2-9]|[JQKA])[CDHS])$";
}

