//
// Created by robert-grigoryan on 5/30/24.
//

#include "PlayerData.h"

bool PlayerData::hasCard(const Card& card) const {
  return hand.at(card.getColor()).contains(card);
}

bool PlayerData::hasColor(const Card::Color color) const{
  return !hand.at(color).empty();
}

std::optional<Error> PlayerData::playNewDeal(const HandConfig& config) {
  clearHand();

  for (Card card: config.getCards()) {
    hand.at(card.getColor()).emplace(card);
  }

  score = 0;

  return std::nullopt;
}

std::optional<Error> PlayerData::takeCard(const Card& card) {
  if (hand.at(card.getColor()).contains(card)) {
    hand.at(card.getColor()).erase(card);
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
    hand.emplace(color, std::unordered_set<Card>());
  }
}

void PlayerData::clearHand() {
  for (Card::Color color : Card::getAllColors()) {
    hand.at(color).clear();
  }
}