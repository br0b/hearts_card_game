//
// Created by robert-grigoryan on 5/27/24.
//

#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>

#include "Error.h"

class Card {
 public:
  enum class Value {
    kTwo = 2,
    kThree = 3,
    kFour = 4,
    kFive = 5,
    kSix = 6,
    kSeven = 7,
    kEight = 8,
    kNine = 9,
    kTen = 10,
    kJack = 11,
    kQueen = 12,
    kKing = 13,
    kAce = 14
  };

  enum class Color { kClub, kDiamond, kHeart, kSpade };

  Card(const Value _value, const Color _color) : value(_value), color(_color) {}
  static std::variant<Card, Error> FromString(const std::string& str);

  [[nodiscard]] Value getValue() const;
  [[nodiscard]] Color getColor() const;
  [[nodiscard]] std::string serialize() const;

  static std::optional<Value> getValueFromString(const std::string &value);
  static std::string getStringFromValue(const Value &value);

  static std::optional<Color> getColorFromChar(char color);
  static char getCharFromColor(const Color &color);

  static std::vector<Color> getAllColors();
  static std::vector<Value> getAllValues();

  bool operator==(const Card &) const;

private:
  Value value;
  Color color;

  static std::unordered_map<std::string, Value> stringToValue;
  static std::unordered_map<Value, std::string> valueToString;

  static std::unordered_map<char, Color> charToColor;
  static std::unordered_map<Color, char> colorToChar;
};

template <>
struct std::hash<Card> {
  std::size_t operator()(const Card& card) const noexcept {
    const std::size_t h1 = std::hash<Card::Value>{}(card.getValue());
    const std::size_t h2 = std::hash<Card::Color>{}(card.getColor());
    // Combine the two hash values
    return h1 ^ (h2 << 1);
  }
};

std::ostream &operator<<(std::ostream &os, const Card &card);

#endif  // CARD_H
