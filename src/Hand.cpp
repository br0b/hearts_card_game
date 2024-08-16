#include <regex>

#include "Hand.h"

void Hand::Parse(std::string str) {
  cards.clear();

  std::regex pattern("(10|[2-9]|[JQKA])([CDHS])");

  for (auto it = std::sregex_iterator(str.begin(), str.end(), pattern);
       it != std::sregex_iterator();
       it++) {
    cards.emplace_back(Card::ParseValue((*it)[1]).value(),
                       Card::ParseColor((*it)[2]).value());
  }
}

const std::vector<Card>& Hand::Get() const {
  return cards;
}

std::ostream& operator<<(std::ostream &os, const Hand &dt) {
  for (const Card &c : dt.cards) {
    os << c;
  }
  return os;
}

