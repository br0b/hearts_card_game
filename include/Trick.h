//
// Created by robert-grigoryan on 5/31/24.
//
#ifndef TRICK_H
#define TRICK_H

#include "Card.h"
#include "Seat.h"

class Trick {
 public:
  void placeCard(const Seat& seat, const Card& card);
  [[nodiscard]] bool isFinished() const;
  [[nodiscard]] Seat getTaker() const;
  [[nodiscard]] std::vector<Card> getCards() const;
  [[nodiscard]] std::string serialize() const;

 private:
  std::vector<Card> cards;
  std::unordered_map<std::string, Seat> ownership;
};

#endif  // TRICK_H
