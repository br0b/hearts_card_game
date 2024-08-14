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

template <typename Iterator>
void Hand::Set(Iterator begin, Iterator end) {
  cards.clear();
  cards.insert(cards.begin(), begin, end);
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

template void Hand::Set<std::array<Card, 4>::iterator>(std::array<Card, 4>::iterator begin, std::array<Card, 4>::iterator end);
template void Hand::Set<std::vector<Card>::iterator>(std::vector<Card>::iterator begin, std::vector<Card>::iterator end);

