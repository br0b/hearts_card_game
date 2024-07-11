//
// Created by robert-grigoryan on 5/27/24.
//
#include <optional>
#include <string>
#include <vector>

#include "Card.h"

std::unordered_map<std::string, Card::Value> Card::stringToValue = {
    {"2", Value::kTwo},   {"3", Value::kThree}, {"4", Value::kFour},
    {"5", Value::kFive},  {"6", Value::kSix},   {"7", Value::kSeven},
    {"8", Value::kEight}, {"9", Value::kNine},  {"10", Value::kTen},
    {"J", Value::kJack},  {"Q", Value::kQueen}, {"K", Value::kKing},
    {"A", Value::kAce}};

std::unordered_map<Card::Value, std::string> Card::valueToString = {
    {Value::kTwo, "2"},   {Value::kThree, "3"}, {Value::kFour, "4"},
    {Value::kFive, "5"},  {Value::kSix, "6"},   {Value::kSeven, "7"},
    {Value::kEight, "8"}, {Value::kNine, "9"},  {Value::kTen, "10"},
    {Value::kJack, "J"},  {Value::kQueen, "Q"}, {Value::kKing, "K"},
    {Value::kAce, "A"}};

std::unordered_map<char, Card::Color> Card::charToColor = {
    {'C', Color::kClub},
    {'D', Color::kDiamond},
    {'H', Color::kHeart},
    {'S', Color::kSpade}};

std::unordered_map<Card::Color, char> Card::colorToChar = {
    {Color::kClub, 'C'},
    {Color::kDiamond, 'D'},
    {Color::kHeart, 'H'},
    {Color::kSpade, 'S'}};

std::variant<Card, Error> Card::FromString(const std::string& str) {
  const std::optional<Value> value = getValueFromString(str.substr(0, str.size() - 1));
  const std::optional<Color> color = getColorFromChar(str[str.size() - 1]);

  if (!value.has_value() || !color.has_value()) {
    return Error(str + " is an invalid card encoding.");
  }

  return Card(value.value(), color.value());
}

Card::Value Card::getValue() const { return value; }

Card::Color Card::getColor() const { return color; }

std::string Card::serialize() const {
  return getStringFromValue(value) + getCharFromColor(color);
}

std::optional<Card::Value> Card::getValueFromString(const std::string &value) {
  if (stringToValue.contains(value)) {
    return stringToValue[value];
  }

  return std::nullopt;
}

std::string Card::getStringFromValue(const Value &value) {
  return valueToString[value];
}

std::optional<Card::Color> Card::getColorFromChar(const char color) {
  if (charToColor.contains(color)) {
    return charToColor[color];
  }

  return std::nullopt;
}

char Card::getCharFromColor(const Color &color) { return colorToChar[color]; }

std::vector<Card::Color> Card::getAllColors() {
  return {Color::kClub, Color::kDiamond, Color::kHeart, Color::kSpade};
}

std::vector<Card::Value> Card::getAllValues() {
  return {Value::kTwo, Value::kThree, Value::kFour,  Value::kFive,
          Value::kSix, Value::kSeven, Value::kEight, Value::kNine,
          Value::kTen, Value::kJack,  Value::kQueen, Value::kKing,
          Value::kAce};
}

bool Card::operator==(const Card& other) const {
  return value == other.value && color == other.color;
}

std::ostream &operator<<(std::ostream &os, const Card &card) {
  os << card.serialize();
  return os;
}
