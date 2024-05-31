//
// Created by robert-grigoryan on 5/31/24.
//

#include <variant>

#include "Error.h"
#include "Game.h"

Seat Game::getCurrentTurn() const { return currentTurn; }

void Game::playNewDeal(const PlayerHandsConfig& config,
                       const DealType& _dealType, const Seat& firstTurn) {
  playersManager.playNewDeal(config);
  currentTurn = firstTurn;
  dealType = _dealType;
  isDealFinished = false;
  history.clear();
}

std::optional<Error> Game::playCard(const Card& card) {
  if (isDealFinished) {
    return Error("The deal is already finished.");
  }

  if (std::optional<Error> error = playersManager.takeCard(currentTurn, card);
      error.has_value()) {
    return error;
  }

  setNextPlayer();
  history.push(currentTurn, card);
  const std::variant<bool, Error> tmp = history.isDealFinished(dealType);

  if (std::holds_alternative<Error>(tmp)) {
    return std::get<Error>(tmp);
  }

  if (std::get<bool>(tmp)) {
    isDealFinished = true;
    distributePoints();
  }

  return std::nullopt;
}

bool Game::isCurrentTrickFinished() const { return history.isTrickFinished(); }

bool Game::isCurrentDealFinished() const { return isDealFinished; }

Game::Game()
    : currentTurn(Seat(Seat::Position::kN)),
      dealType(DealType::Type::kTricksBad),
      isDealFinished(false) {}

std::optional<Error> Game::setNextPlayer() {
  if (history.isTrickFinished()) {
    const std::variant<Seat, Error> tmp = history.getTrickTaker();
    if (std::holds_alternative<Error>(tmp)) {
      return std::get<Error>(tmp);
    }
    currentTurn = std::get<Seat>(tmp);
  } else {
    const char tmp = currentTurn.serialize();
    if (!turnProgression.contains(tmp)) {
      return Error("The provided seat is not supported in turn progression.");
    }
    currentTurn = turnProgression.at(tmp);
  }

  return std::nullopt;
}

std::optional<Error> Game::distributePoints() {
  std::variant<std::unordered_map<char, int>, Error> points =
      history.getPoints(dealType);
  if (std::holds_alternative<Error>(points)) {
    return std::get<Error>(points);
  }
  return playersManager.distributePoints(
      std::get<std::unordered_map<char, int>>(points));
}

const std::unordered_map<char, Seat> Game::turnProgression = {
    {'N', Seat(Seat::Position::kE)},
    {'E', Seat(Seat::Position::kS)},
    {'S', Seat(Seat::Position::kW)},
    {'W', Seat(Seat::Position::kN)},
};
