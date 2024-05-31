//
// Created by robert-grigoryan on 5/30/24.
//

#include "PlayerData.h"

bool PlayerData::hasCard(const Card& card) const {
  return hand.contains(card.serialize());
}

void PlayerData::playNewDeal(const HandConfig& config) {
  hand = config.getCards();
  score = 0;
}

std::optional<Error> PlayerData::takeCard(const Card& card) {
  if (hand.contains(card.serialize())) {
    hand.erase(card.serialize());
    return std::nullopt;
  }

  return Error("Card not in player's hand.");
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

PlayerData::PlayerData() : score(0), total(0) {}