//
// Created by robert-grigoryan on 5/31/24.
//

#ifndef GAME_H
#define GAME_H
#include <PlayersManager.h>

#include "DealConfig.h"
#include "DealType.h"
#include "TrickHistory.h"

class Game {
 public:
  Seat getCurrentTurn() const;
  int getCurrentTrickNumber() const;
  void playNewDeal(const DealConfig& dealConfig);
  std::optional<Error> playCard(const Card& card);
  bool isCurrentTrickFinished() const;
  bool isCurrentDealFinished() const;

  Game();

 private:
  std::optional<Error> setNextPlayer();
  std::optional<Error> distributePoints();

  PlayersManager playersManager;
  Seat currentTurn;
  DealType dealType;
  bool isDealFinished;
  TrickHistory history;

  static const std::unordered_map<Seat::Position, Seat> turnProgression;
};

#endif  // GAME_H
