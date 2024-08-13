#include "MessageDeal.h"
#include "DealType.h"
#include "MaybeError.h"

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

std::ostream& operator<<(std::ostream &os, const MessageDeal &msg) {
  os << "DEAL" << msg.type << msg.first << msg.hand;
  return os;
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

std::string MessageDeal::GetPattern() {
  return "^DEAL(([1-7])([NESW])((?:(?:10|[2-9]|[JQKA])[CDHS]){13})$";
}

