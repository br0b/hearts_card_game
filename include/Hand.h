#ifndef HAND_H
#define HAND_H

#include <vector>

#include "Card.h"

class Hand {
 public:
  // Get all cards from string.
  void Parse(std::string str);

  template <typename Iterator>
  void Set(Iterator begin, Iterator end);

  [[nodiscard]] const std::vector<Card>& Get() const;

  friend std::ostream& operator<<(std::ostream &os, const Hand &dt);

 private:
  std::vector<Card> cards;
};

#endif  // HAND_H
