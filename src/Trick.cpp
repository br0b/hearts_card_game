//
// Created by robert-grigoryan on 5/31/24.
//
#include <iomanip>

#include "Trick.h"

void Trick::placeCard(const Seat& seat, const Card& card) {
  ownership.emplace(card.serialize(), seat);
  cards.push_back(card);
}

bool Trick::isFinished() const {
  return cards.size() == 4;
}

Seat Trick::getTaker() const {
  const Card::Color leadingColor = cards[0].getColor();
  Card bestCard = cards[0];

  for (Card card : cards) {
    if (card.getColor() == leadingColor
        && card.getValue() > bestCard.getValue()) {
      bestCard = card;
    }
  }

  return ownership.at(bestCard.serialize());
}

std::vector<Card> Trick::getCards() const{
  return cards;
}

std::string Trick::serialize() const {
  std::ostringstream os;
  for (const Card& card : cards) {
    os << card.serialize();
  }
  return os.str();
}
