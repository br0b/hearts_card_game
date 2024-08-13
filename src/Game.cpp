#include "Game.h"
#include "DealType.h"

Game::Game() : currentTrick{{}, 1} {}

std::optional<Game::TrickResult> Game::Play(Card card) {
  currentTrick.cards.push_back(card);
  hands[static_cast<size_t>(currentTurn.Get())][card.GetColorIndex()]
    .erase(card);
  currentTurn.CycleClockwise();

  if (currentTrick.cards.size() < 4) {
    return std::nullopt;
  }

  // Trick finished.
  TrickResult res;
  res.taker = GetTaker();
  res.points = trickJudge(currentTrick);
  currentTurn = res.taker;
  currentTrick.cards.clear();
  currentTrick.number++;
  return res;
}

void Game::Deal(DealConfig &config) {
  currentTurn = config.GetFirst();
  currentTrick.cards.clear();
  currentTrick.number = 1;

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
}

bool Game::IsMoveLegal(Seat player, Card card) const {
  if (!hands[player.GetIndex()][card.GetColorIndex()].contains(card)) {
    return false;
  }

  if (currentTrick.cards.empty()) {
    return true;
  }

  Card firstCard = currentTrick.cards.back();
  return card.GetColor() == firstCard.GetColor()
         || hands[player.GetIndex()][firstCard.GetColorIndex()].empty();
}

Seat Game::GetTaker() const {
  auto card = currentTrick.cards.begin();
  Card first = *card;
  Seat player = currentTurn;
  Seat taker = player;
  card++;
  player.CycleClockwise();

  for (; card != currentTrick.cards.end(); card++) {
    if (card->GetColor() == first.GetColor()
        && card->GetValue() > first.GetValue()) {
      taker = player;
    }
    player.CycleClockwise();
  }

  return taker;
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
  int n = trick.number;
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

