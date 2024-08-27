#ifndef HAND_H
#define HAND_H

#include <vector>

#include "Card.h"

class Hand {
 public:
  // Get all cards from string.
  void Parse(std::string str);

  template <typename T>
  void Set(T cards_) {
    cards.clear();
    cards.insert(cards.end(), cards_.begin(), cards_.end());
  }

  [[nodiscard]] const std::vector<Card>& Get() const;

  friend std::ostream& operator<<(std::ostream &os, const Hand &dt);

 private:
  std::vector<Card> cards;
};

#endif  // HAND_H
