#include <sstream>

#include "MessageDeal.h"
#include "DealType.h"
#include "MaybeError.h"
#include "Utilities.h"

void MessageDeal::SetType(DealType type_) {
  type = type_;
}

void MessageDeal::SetFirst(Seat first_) {
  first = first_;
}

void MessageDeal::SetHand(Hand hand_) {
  hand = hand_;
}

DealType MessageDeal::GetType() const {
  return type;
}

Seat MessageDeal::GetFirst() const {
  return first;
}

const Hand& MessageDeal::GetHand() const {
  return hand;
}

std::string MessageDeal::Str() const {
  std::ostringstream oss;
  oss << "DEAL" << type << first << hand;
  return oss.str();
}

std::optional<std::string> MessageDeal::UserStr() const {
  std::ostringstream oss;
  oss << "New deal " << type << ": starting place "
      << first << ", your cards: ";
  Utilities::StrList(oss, hand.Get().begin(), hand.Get().end());
  oss << '.';
  return oss.str();
}

MaybeError MessageDeal::SetAfterMatch(std::smatch match) {
  MaybeError error;

  if (error = type.Parse(match[1]); error.has_value()) {
    return error;
  }

  if (error = first.Parse(match[2]); error.has_value()) {
    return error;
  }

  hand.Parse(match[3]);

  return std::nullopt;
}

std::string MessageDeal::GetPattern() const {
  return "^DEAL([1-7])([NESW])((?:(?:10|[2-9]|[JQKA])[CDHS]){13})$";
}

