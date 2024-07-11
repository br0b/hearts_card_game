//
// Created by robert-grigoryan on 5/31/24.
//
#include <variant>

#include "Error.h"
#include "Game.h"

Seat Game::getCurrentTurn() const { return currentTurn; }

int Game::getCurrentTrickNumber() const {
  return history.getCurrentTrickNumber();
}

void Game::playNewDeal(const DealConfig& dealConfig) {
  playersManager.playNewDeal(dealConfig.getHandsConfig());
  currentTurn = Seat(dealConfig.getFirstPlayer());
  dealType = dealConfig.getDealType();
  isDealFinished = false;
  history.clear();
}

std::optional<Error> Game::playCard(const Card& card) {
  if (isDealFinished) {
    return Error("The deal is already finished.");
  }

  if (const std::optional<Card::Color> leadingColor = history.getLeadingColor();
      leadingColor.has_value() && card.getColor() != leadingColor &&
      playersManager.hasColor(currentTurn, leadingColor.value())) {
    return Error("A player must follow the leading color.");
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
    currentTurn = turnProgression.at(currentTurn.getPosition());
  }

  return std::nullopt;
}

std::optional<Error> Game::distributePoints() {
  std::variant<std::unordered_map<Seat::Position, int>, Error> points =
      history.getPoints(dealType);
  if (std::holds_alternative<Error>(points)) {
    return std::get<Error>(points);
  }
  return playersManager.distributePoints(
      std::get<std::unordered_map<Seat::Position, int>>(points));
}

const std::unordered_map<Seat::Position, Seat> Game::turnProgression = {
    {Seat::Position::kN, Seat(Seat::Position::kE)},
    {Seat::Position::kE, Seat(Seat::Position::kS)},
    {Seat::Position::kS, Seat(Seat::Position::kW)},
    {Seat::Position::kW, Seat(Seat::Position::kN)},
};
