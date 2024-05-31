//
// Created by robert-grigoryan on 5/30/24.
//

#include "PlayerData.h"

bool PlayerData::hasCard(const Card& card) const {
  return hand.at(card.getColor()).contains(card.serialize());
}

bool PlayerData::hasColor(const Card::Color color) const{
  return !hand.at(color).empty();
}

std::optional<Error> PlayerData::playNewDeal(const HandConfig& config) {
  clearHand();

  for (const std::string& cardStr : config.getCards()) {
    std::variant<Card, Error> _card = Card::FromString(cardStr);
    if (std::holds_alternative<Error>(_card)) {
      clearHand();
      return std::get<Error>(_card);
    }
    Card card = std::get<Card>(_card);
    hand.at(card.getColor()).emplace(cardStr);
  }

  score = 0;

  return std::nullopt;
}

std::optional<Error> PlayerData::takeCard(const Card& card) {
  if (hand.at(card.getColor()).contains(card.serialize())) {
    hand.at(card.getColor()).erase(card.serialize());
    return std::nullopt;
  }

  return Error("Card " + card.serialize() + " not in player's hand.");
}

int PlayerData::getScore() const {
  return score;
}

int PlayerData::getTotal() const {
  return total;
}

void PlayerData::givePoints(int points) {
  score += points;
  total += points;
}

PlayerData::PlayerData() : score(0), total(0) {
  for (Card::Color color : Card::getAllColors()) {
    hand.emplace(color, std::unordered_set<std::string>());
  }
}

void PlayerData::clearHand() {
  for (Card::Color color : Card::getAllColors()) {
    hand.at(color).clear();
  }
}