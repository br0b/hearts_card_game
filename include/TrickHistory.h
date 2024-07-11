//
// Created by robert-grigoryan on 5/31/24.
//
#ifndef TRICKHISTORY_H
#define TRICKHISTORY_H

#include <functional>
#include <variant>
#include <vector>

#include "Card.h"
#include "DealType.h"
#include "Error.h"
#include "Seat.h"
#include "Trick.h"

class TrickHistory {
 public:
  void push(Seat seat, Card card);
  [[nodiscard]] bool isTrickFinished() const;

  [[nodiscard]] std::variant<bool, Error> isDealFinished(
      DealType dealType) const;
  [[nodiscard]] int getCurrentTrickNumber() const;
  [[nodiscard]] Trick getCurrentTrick() const;
  [[nodiscard]] std::optional<Card::Color> getLeadingColor() const;
  [[nodiscard]] std::variant<Seat, Error> getTrickTaker() const;
  [[nodiscard]] std::variant<std::unordered_map<Seat::Position, int>, Error>
  getPoints(DealType dealType) const;
  void clear();

  TrickHistory();

 private:
  std::vector<Trick> history;
  [[nodiscard]] static bool _isTrickFinished(const std::vector<Trick>& history);

  static bool isDealFinishedTricksBad(const std::vector<Trick>& tricks);
  static bool isDealFinishedHeartsBad(const std::vector<Trick>& tricks);
  static bool isDealFinishedQueensBad(const std::vector<Trick>& tricks);
  static bool isDealFinishedGentlemenBad(const std::vector<Trick>& tricks);
  static bool isDealFinishedKingOfHeartsBad(const std::vector<Trick>& tricks);
  static bool isDealFinishedSeventhAndLastTrickBad(
      const std::vector<Trick>& tricks);
  static bool isDealFinishedRobber(const std::vector<Trick>& tricks);

  static std::unordered_map<Seat::Position, int> pointDistributionTricksBad(
      const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int> pointDistributionHeartsBad(
      const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int> pointDistributionQueensBad(
      const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int> pointDistributionGentlemenBad(
      const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int>
  pointDistributionKingOfHeartsBad(const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int>
  pointDistributionSeventhAndLastTrickBad(const std::vector<Trick>& tricks);
  static std::unordered_map<Seat::Position, int> pointDistributionRobber(
      const std::vector<Trick>& tricks);

  static std::unordered_map<int, std::function<bool(std::vector<Trick>)>>
      dealFinishedFunctions;
  static std::unordered_map<int, std::function<std::unordered_map<
                                     Seat::Position, int>(std::vector<Trick>)>>
      pointDistributionFunctions;

  static std::unordered_map<Seat::Position, int> getEmptyPointDistribution();
};

#endif  // TRICKHISTORY_H
