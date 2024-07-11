//
// Created by robert-grigoryan on 5/31/24.
//
#include "TrickHistory.h"

std::unordered_map<int, std::function<bool(std::vector<Trick>)>>
    TrickHistory::dealFinishedFunctions = {
        {DealType(DealType::Type::kTricksBad).serialize(),
         isDealFinishedTricksBad},
        {DealType(DealType::Type::kHeartsBad).serialize(),
         isDealFinishedHeartsBad},
        {DealType(DealType::Type::kQueensBad).serialize(),
         isDealFinishedQueensBad},
        {DealType(DealType::Type::kGentlemenBad).serialize(),
         isDealFinishedGentlemenBad},
        {DealType(DealType::Type::kKingOfHeartsBad).serialize(),
         isDealFinishedKingOfHeartsBad},
        {DealType(DealType::Type::kSeventhAndLastTrickBad).serialize(),
         isDealFinishedSeventhAndLastTrickBad},
        {DealType(DealType::Type::kRobber).serialize(), isDealFinishedRobber}};

std::unordered_map<int, std::function<std::unordered_map<Seat::Position, int>(
                            std::vector<Trick>)>>
    TrickHistory::pointDistributionFunctions = {
        {DealType(DealType::Type::kTricksBad).serialize(),
         pointDistributionTricksBad},
        {DealType(DealType::Type::kHeartsBad).serialize(),
         pointDistributionHeartsBad},
        {DealType(DealType::Type::kQueensBad).serialize(),
         pointDistributionQueensBad},
        {DealType(DealType::Type::kGentlemenBad).serialize(),
         pointDistributionGentlemenBad},
        {DealType(DealType::Type::kKingOfHeartsBad).serialize(),
         pointDistributionKingOfHeartsBad},
        {DealType(DealType::Type::kSeventhAndLastTrickBad).serialize(),
         pointDistributionSeventhAndLastTrickBad},
        {DealType(DealType::Type::kRobber).serialize(),
         pointDistributionRobber}};

void TrickHistory::push(const Seat seat, const Card card) {
  if (history.empty() || _isTrickFinished(history)) {
    history.emplace_back();
  }

  history.back().placeCard(seat, card);
}

bool TrickHistory::isTrickFinished() const { return _isTrickFinished(history); }

bool TrickHistory::_isTrickFinished(const std::vector<Trick>& history) {
  return history.back().isFinished();
}

std::variant<bool, Error> TrickHistory::isDealFinished(
    const DealType dealType) const {
  if (!dealFinishedFunctions.contains(dealType.serialize())) {
    return Error("The provided deal type is not supported.");
  }

  return dealFinishedFunctions.at(dealType.serialize())(history);
}

int TrickHistory::getCurrentTrickNumber() const {
  return static_cast<int>(history.size());
}

Trick TrickHistory::getCurrentTrick() const {
  return history.back();
}

std::optional<Card::Color> TrickHistory::getLeadingColor() const {
  if (!history.empty() && !isTrickFinished()) {
    return history.back().getCards()[0].getColor();
  }

  return std::nullopt;
}

std::variant<Seat, Error> TrickHistory::getTrickTaker() const {
  if (!_isTrickFinished(history)) {
    return Error("The trick is not finished yet.");
  }

  return history.back().getTaker();
}

std::variant<std::unordered_map<Seat::Position, int>, Error>
TrickHistory::getPoints(const DealType dealType) const {
  if (!pointDistributionFunctions.contains(dealType.serialize())) {
    return Error("The provided deal type is not supported.");
  }

  return pointDistributionFunctions.at(dealType.serialize())(history);
}

void TrickHistory::clear() { history.clear(); }

TrickHistory::TrickHistory() : history({Trick()}) {}

bool TrickHistory::isDealFinishedTricksBad(const std::vector<Trick>& tricks) {
  return _isTrickFinished(tricks) && tricks.size() == 13;
}

bool TrickHistory::isDealFinishedHeartsBad(const std::vector<Trick>& tricks) {
  if (!_isTrickFinished(tricks)) {
    return false;
  }

  int nHearts = 0;
  for (const Trick& trick : tricks) {
    for (const Card& card : trick.getCards()) {
      if (card.getColor() == Card::Color::kHeart) {
        nHearts++;
      }
    }
  }

  return nHearts == 13;
}

bool TrickHistory::isDealFinishedQueensBad(const std::vector<Trick>& tricks) {
  if (!_isTrickFinished(tricks)) {
    return false;
  }

  int nQueens = 0;
  for (const Trick& trick : tricks) {
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kQueen) {
        nQueens++;
      }
    }
  }

  return nQueens == 4;
}

bool TrickHistory::isDealFinishedGentlemenBad(
    const std::vector<Trick>& tricks) {
  if (_isTrickFinished(tricks)) {
    return false;
  }

  int nGentlemen = 0;
  for (const Trick& trick : tricks) {
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kJack ||
          card.getValue() == Card::Value::kKing) {
        nGentlemen++;
      }
    }
  }

  return nGentlemen == 8;
}

bool TrickHistory::isDealFinishedKingOfHeartsBad(
    const std::vector<Trick>& tricks) {
  if (!_isTrickFinished(tricks)) {
    return false;
  }

  for (const Trick& trick : tricks) {
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kKing &&
          card.getColor() == Card::Color::kHeart) {
        return true;
      }
    }
  }

  return false;
}

bool TrickHistory::isDealFinishedSeventhAndLastTrickBad(
    const std::vector<Trick>& tricks) {
  return isDealFinishedTricksBad(tricks);
}

bool TrickHistory::isDealFinishedRobber(const std::vector<Trick>& tricks) {
  return isDealFinishedTricksBad(tricks);
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionTricksBad(const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  for (const Trick& trick : tricks) {
    points.at(trick.getTaker().getPosition())++;
  }

  return points;
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionHeartsBad(const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  for (const Trick& trick : tricks) {
    Seat::Position taker = trick.getTaker().getPosition();

    for (const Card& card : trick.getCards()) {
      if (card.getColor() == Card::Color::kHeart) {
        points.at(taker)++;
      }
    }
  }

  return points;
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionQueensBad(const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  for (const Trick& trick : tricks) {
    Seat::Position taker = trick.getTaker().getPosition();
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kQueen) {
        points.at(taker) += 5;
      }
    }
  }

  return points;
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionGentlemenBad(const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  for (const Trick& trick : tricks) {
    Seat::Position taker = trick.getTaker().getPosition();
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kJack ||
          card.getValue() == Card::Value::kKing) {
        points.at(taker) += 2;
      }
    }
  }

  return points;
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionKingOfHeartsBad(
    const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  for (const Trick& trick : tricks) {
    Seat::Position taker = trick.getTaker().getPosition();
    for (const Card& card : trick.getCards()) {
      if (card.getValue() == Card::Value::kKing &&
          card.getColor() == Card::Color::kHeart) {
        points.at(taker) += 18;
      }
    }
  }

  return points;
}

std::unordered_map<Seat::Position, int>
TrickHistory::pointDistributionSeventhAndLastTrickBad(
    const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> points = getEmptyPointDistribution();

  points.at(tricks[6].getTaker().getPosition()) = 1;
  points.at(tricks.back().getTaker().getPosition()) = 1;

  return points;
}

std::unordered_map<Seat::Position, int> TrickHistory::pointDistributionRobber(
    const std::vector<Trick>& tricks) {
  std::unordered_map<Seat::Position, int> distribution =
      getEmptyPointDistribution();
  std::vector<DealType> tmp = DealType::getAllTypes();
  std::vector<DealType> dealtypes;
  std::copy_if(tmp.begin(), tmp.end(), std::back_inserter(dealtypes),
               [](const DealType& dealType) {
                 return dealType.getType() != DealType::Type::kRobber;
               });

  for (DealType dealType : dealtypes) {
    for (const auto& [seat, points] :
         pointDistributionFunctions.at(dealType.serialize())(tricks)) {
      distribution.at(seat) += points;
    }
  }

  return distribution;
}

std::unordered_map<Seat::Position, int>
TrickHistory::getEmptyPointDistribution() {
  std::unordered_map<Seat::Position, int> dist;
  for (Seat seat : Seat::getAllSeats()) {
    dist.emplace(seat.getPosition(), 0);
  }
  return dist;
}
