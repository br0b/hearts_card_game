//
// Created by robert-grigoryan on 5/30/24.
//
#include "PlayersManager.h"

PlayersManager::PlayersManager() {
  for (const Seat seat : Seat::getAllSeats()) {
    players.emplace(seat.getPosition(), PlayerData());
  }
}

void PlayersManager::playNewDeal(const PlayerHandsConfig& config) {
  for (Seat seat : Seat::getAllSeats()) {
    players.at(seat.getPosition())
        .playNewDeal(config.getHandConfig(seat.getPosition()));
  }
}

bool PlayersManager::hasCard(const Seat seat, const Card& card) const {
  return players.at(seat.getPosition()).hasCard(card);
}

bool PlayersManager::hasColor(const Seat seat, Card::Color color) const {
  return players.at(seat.getPosition()).hasColor(color);
}

std::optional<Error> PlayersManager::takeCard(const Seat seat,
                                              const Card& card) {
  return players.at(seat.getPosition()).takeCard(card);
}

int PlayersManager::getScore(const Seat seat) const {
  return players.at(seat.getPosition()).getScore();
}

int PlayersManager::getTotal(const Seat seat) const {
  return players.at(seat.getPosition()).getTotal();
}

std::optional<Error> PlayersManager::distributePoints(
    const std::unordered_map<Seat::Position, int>& points) {
  for (const Seat seat : Seat::getAllSeats()) {
    if (!points.contains(seat.getPosition())) {
      return Error("Points for all players are not provided.");
    }

    players.at(seat.getPosition()).givePoints(points.at(seat.getPosition()));
  }

  return std::nullopt;
}
