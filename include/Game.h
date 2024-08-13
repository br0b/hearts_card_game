#ifndef GAME_H
#define GAME_H

#include <functional>
#include <unordered_set>

#include "Card.h"
#include "DealConfig.h"

class Game {
 public:
  struct Trick {
    std::vector<Card> cards;
    int number;
  };

  struct TrickResult {
    Seat taker;
    int points;
  };

  Game();

  std::optional<TrickResult> Play(Card card);
  void Deal(DealConfig &config);

  // Assumes there is an ongoing deal.
  // Move is legal when the player has the card and
  // one of the following is true:
  //   1. This is the first card in the trick,
  //   2. This card is in the same color as the color of
  //      the first card in the trick.
  //   3. The player doesn't have any cards in the color of
  //      the first card in the trick.
  [[nodiscard]] bool IsMoveLegal(Seat player, Card card) const;

 private:
  // Assumes currentTurn is equal to the first player of the trick.
  [[nodiscard]] Seat GetTaker() const;

  [[nodiscard]] static int TricksBadJudge(const Trick &trick);

  [[nodiscard]] static int HeartsBadJudge(const Trick &trick);

  [[nodiscard]] static int QueensBadJudge(const Trick &trick);

  [[nodiscard]] static int GentlemenBadJudge(const Trick &trick);

  [[nodiscard]] static int KingOfHeartsBadJudge(const Trick &trick);

  [[nodiscard]] static int SeventhAndLastTrickBadJudge(const Trick &trick);

  [[nodiscard]] static int RobberJudge(const Trick &trick);

  Seat currentTurn;
  Trick currentTrick;
  std::array<std::array<std::unordered_set<Card>, 4>, 4> hands;
  std::function<int(const Trick&)> trickJudge;
};

#endif  // GAME_H

