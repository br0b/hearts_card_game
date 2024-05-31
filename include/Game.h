//
// Created by robert-grigoryan on 5/31/24.
//

#ifndef GAME_H
#define GAME_H
#include <PlayersManager.h>

#include "TrickHistory.h"
#include "DealType.h"

class Game {
 public:
  Seat getCurrentTurn() const;
  void playNewDeal(const PlayerHandsConfig& config, const DealType& _dealType,
                   const Seat& firstTurn);
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

  static const std::unordered_map<char, Seat> turnProgression;
};

#endif  // GAME_H
