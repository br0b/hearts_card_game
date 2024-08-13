#ifndef CARD_STORE_H
#define CARD_STORE_H

#include <vector>

#include "Card.h"
#include "MaybeError.h"

class CardStore {
 public:
  CardStore();

  [[nodiscard]] MaybeError Push(Card card);
  [[nodiscard]] MaybeError Pop(Card card);
  void Clear();

 private:
  
  std::vector<std::vector<Card::Value>> cards;
};

#endif  // CARD_STORE_H
