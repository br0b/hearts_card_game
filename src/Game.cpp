#include <memory>
#include <optional>

#include "DealType.h"
#include "Error.h"
#include "MaybeError.h"
#include "TrickNumber.h"
#include "Game.h"

MaybeError Game::Play(Card card, std::optional<Game::TrickResult> &result) {
  if (!currentTrick.has_value()) {
    return NotStarted("Game::Play");
  } else if (!hands[currentTrick.value().turn.GetIndex()][card.GetColorIndex()]
      .contains(card)) {
    return std::make_unique<Error>("Game::Play",
                                   "Player doesn't have the card.");
  }

  int trickNumber = currentTrick.value().number.Get();

  currentTrick.value().cards.push_back(card);
  hands[currentTrick.value().turn.GetIndex()][card.GetColorIndex()]
    .erase(card);
  currentTrick.value().turn.CycleClockwise();

  if (currentTrick.value().cards.size() < 4) {
    result = std::nullopt;
    return std::nullopt;
  }

  // Trick finished.
  result = TrickResult();
  std::copy_n(currentTrick.value().cards.begin(), 3,
              result.value().cards.begin());
  result.value().cards.back() = card;
  result.value().points = trickJudge(currentTrick.value());
  if (MaybeError error = GetTaker(result.value().taker); error.has_value()) {
    return error;
  }
  
  if (trickNumber < 13) {
    currentTrick.value().turn = result.value().taker;
    currentTrick.value().cards.clear();
    if (MaybeError error = currentTrick.value().number.Set(trickNumber + 1);
        error.has_value()) {
      return error;
    }
  } else {
    currentTrick.reset();
  }

  return std::nullopt;
}

MaybeError Game::Deal(DealConfig &config) {
  if (currentTrick.has_value()) {
    return std::make_unique<Error>("Game::Deal", "There is an ongoing deal.");
  }

  TrickNumber trickNumber;
  if (MaybeError error = trickNumber.Set(1); error.has_value()) {
    return error;
  }

  currentTrick = std::make_optional<Trick>({{}, trickNumber,
                                           config.GetFirst()});

  for (size_t i = 0; i < 4; i++) {
    for (const Card &c : config.GetHands()[i].Get()) {
      hands[i][c.GetColorIndex()].insert(c);
    }
  }

  switch (config.GetType().Get()) {
    case DealType::Value::kTricksBad: {
      trickJudge = TricksBadJudge;
      break;
    }
    case DealType::Value::kHeartsBad: {
      trickJudge = HeartsBadJudge;
      break;
    }
    case DealType::Value::kQueensBad: {
      trickJudge = QueensBadJudge;
      break;
    }
    case DealType::Value::kGentlemenBad: {
      trickJudge = GentlemenBadJudge;
      break;
    }
    case DealType::Value::kKingOfHeartsBad: {
      trickJudge = KingOfHeartsBadJudge;
      break;
    }
    case DealType::Value::kSeventhAndLastTrickBad: {
      trickJudge = KingOfHeartsBadJudge;
      break;
    }
    case DealType::Value::kRobber: {
      trickJudge = KingOfHeartsBadJudge;
      break;
    }
  };

  return std::nullopt;
}

bool Game::IsMoveLegal(Seat player, Card card) const {
  if (!currentTrick.has_value()
      || !hands[player.GetIndex()][card.GetColorIndex()].contains(card)) {
    return false;
  }

  if (currentTrick.value().cards.empty()) {
    return true;
  }

  Card firstCard = currentTrick.value().cards.front();
  return card.GetColor() == firstCard.GetColor()
         || hands[player.GetIndex()][firstCard.GetColorIndex()].empty();
}

const std::optional<Game::Trick> &Game::GetCurrentTrick() const {
  return currentTrick;
}

MaybeError Game::GetTaker(Seat &taker) const {
  if (!currentTrick.has_value()) {
    return NotStarted("Game::GetTaker");
  }

  auto first = currentTrick.value().cards.begin();
  Seat player = currentTrick.value().turn;
  taker = player;
  player.CycleClockwise();

  for (auto card = first + 1; card != currentTrick.value().cards.end();
       card++) {
    if (card->GetColor() == first->GetColor()
        && card->GetValue() > first->GetValue()) {
      taker = player;
    }
    player.CycleClockwise();
  }

  return std::nullopt;
}

int Game::TricksBadJudge(const Game::Trick &trick) {
  (void)trick;
  return 1;
}

int Game::HeartsBadJudge(const Game::Trick &trick) {
  int n = 0;

  for (const Card &c: trick.cards) {
    if (c.GetColor() == Card::Color::kHeart) {
      n++;
    }
  }

  return n;
}

int Game::QueensBadJudge(const Game::Trick &trick) {
  int n= 0;

  for (const Card &c: trick.cards) {
    if (c.GetValue() == Card::Value::kQ) {
      n++;
    }
  }

  return 5 * n;
}

int Game::GentlemenBadJudge(const Game::Trick &trick) {
  int n = 0;

  for (const Card &c: trick.cards) {
    if (c.GetValue() == Card::Value::kJ || c.GetValue() == Card::Value::kK) {
      n++;
    }
  }

  return 2 * n;
}

int Game::KingOfHeartsBadJudge(const Game::Trick &trick) {
  int n = 0;
  Card kingOfHearts(Card::Value::kK, Card::Color::kHeart);

  for (const Card &c: trick.cards) {
    if (c == kingOfHearts) {
      n++;
    }
  }

  return 18 * n;
}

int Game::SeventhAndLastTrickBadJudge(const Game::Trick &trick) {
  int n = trick.number.Get();
  return (n == 7 || n == 13) ? 10 : 0;
}

int Game::RobberJudge(const Game::Trick &trick) {
  return TricksBadJudge(trick)
    + HeartsBadJudge(trick)
    + QueensBadJudge(trick)
    + GentlemenBadJudge(trick)
    + GentlemenBadJudge(trick)
    + KingOfHeartsBadJudge(trick)
    + SeventhAndLastTrickBadJudge(trick);
}

std::unique_ptr<Error> Game::NotStarted(std::string funName) {
  return std::make_unique<Error>(std::move(funName),
                                 "Game hasn't started yet.");
}

