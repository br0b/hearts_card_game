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

  [[nodiscard]] Value getValue() const;
  [[nodiscard]] Color getColor() const;
  [[nodiscard]] std::string serialize() const;

  static std::optional<Value> getValueFromString(const std::string &value);
  static std::string getStringFromValue(const Value &value);

  static std::optional<Color> getColorFromChar(char color);
  static char getCharFromColor(const Color &color);

  static std::vector<Color> getAllColors();
  static std::vector<Value> getAllValues();

private:
  Value value;
  Color color;

  static std::unordered_map<std::string, Value> stringToValue;
  static std::unordered_map<Value, std::string> valueToString;

  static std::unordered_map<char, Color> charToColor;
  static std::unordered_map<Color, char> colorToChar;
};

std::ostream &operator<<(std::ostream &os, const Card &card);

#endif  // CARD_H
