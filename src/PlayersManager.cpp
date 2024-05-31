//
// Created by robert-grigoryan on 5/30/24.
//
#include "PlayersManager.h"

PlayersManager::PlayersManager() {
  players[Seat(Seat::Position::kN).serialize()] = PlayerData();
}

void PlayersManager::playNewDeal(const PlayerHandsConfig& config) {
  for (Seat seat : Seat::getAllSeats()) {
    players.at(seat.serialize()).playNewDeal(config.getHandConfig(seat));
  }
}

bool PlayersManager::hasCard(const Seat seat, const Card& card) const {
  return players.at(seat.serialize()).hasCard(card);
}

std::optional<Error> PlayersManager::takeCard(const Seat seat,
                                              const Card& card) {
  return players.at(seat.serialize()).takeCard(card);
}

int PlayersManager::getScore(const Seat seat) const {
  return players.at(seat.serialize()).getScore();
}

int PlayersManager::getTotal(const Seat seat) const {
  return players.at(seat.serialize()).getTotal();
}

std::optional<Error> PlayersManager::distributePoints(
    const std::unordered_map<char, int>& points) {
  for (const Seat seat : Seat::getAllSeats()) {
    if (!points.contains(seat.serialize())) {
      return Error("Points for all players are not provided.");
    }

    players.at(seat.serialize()).givePoints(points.at(seat.serialize()));
  }

  return std::nullopt;
}
