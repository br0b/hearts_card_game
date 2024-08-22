#ifndef GAME_H
#define GAME_H

#include <functional>
#include <memory>
#include <unordered_set>

#include "Card.h"
#include "DealConfig.h"
#include "MaybeError.h"
#include "TrickNumber.h"

class Game {
 public:
  struct Trick {
    std::vector<Card> cards;
    TrickNumber number;
    Seat turn;
  };

  struct TrickResult {
    std::array<Card, 4> cards;
    Seat taker;
    int points;
  };

  // If this completes the current trick,
  // then on return the argument result is set.
  [[nodiscard]] MaybeError Play(Card card, std::optional<TrickResult> &result);
  [[nodiscard]] MaybeError Deal(DealConfig &config);

  // Assumes there is an ongoing deal.
  // Move is legal when the player has the card and
  // one of the following is true:
  //   1. This is the first card in the trick,
  //   2. This card is in the same color as the color of
  //      the first card in the trick.
  //   3. The player doesn't have any cards in the color of
  //      the first card in the trick.
  [[nodiscard]] bool IsMoveLegal(Seat player, Card card) const;

  [[nodiscard]] const std::optional<Trick> &GetCurrentTrick() const;

  [[nodiscard]] static std::unique_ptr<Error> ErrorNotStarted(std::string funName);

 private:
  // Assumes currentTurn is equal to the first player of the trick.
  [[nodiscard]] MaybeError GetTaker(Seat &taker) const;

  [[nodiscard]] static int TricksBadJudge(const Trick &trick);

  [[nodiscard]] static int HeartsBadJudge(const Trick &trick);

  [[nodiscard]] static int QueensBadJudge(const Trick &trick);

  [[nodiscard]] static int GentlemenBadJudge(const Trick &trick);

  [[nodiscard]] static int KingOfHeartsBadJudge(const Trick &trick);

  [[nodiscard]] static int SeventhAndLastTrickBadJudge(const Trick &trick);

  [[nodiscard]] static int RobberJudge(const Trick &trick);

  // If currentTrick.number = 0 me
  std::optional<Trick> currentTrick;
  std::array<std::array<std::unordered_set<Card>, 4>, 4> hands;
  std::function<int(const Trick&)> trickJudge;
};

#endif  // GAME_H

