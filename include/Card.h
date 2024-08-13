#ifndef CARD_H
#define CARD_H

#include <functional>
#include <optional>
#include <ostream>

class Card {
 public:
  enum class Value {
    k2 = 0,
    k3,
    k4,
    k5,
    k6,
    k7,
    k8,
    k9,
    k10,
    kJ,
    kQ,
    kK,
    kA,
  };

  enum class Color {
    kCub = 0,
    kDiamond,
    kHeart,
    kSpade,
  };
  
  Card(Value value, Color color);
  
  [[nodiscard]] static std::optional<Value> ParseValue(std::string value);
  [[nodiscard]] static std::optional<Color> ParseColor(std::string color);

  [[nodiscard]] Value GetValue() const;
  [[nodiscard]] Color GetColor() const;
  [[nodiscard]] int GetColorIndex() const;

  bool operator==(const Card &other) const;
  friend std::ostream& operator<<(std::ostream &os, const Card &c);

 private:
  Value value;
  Color color;
};

template <>
struct std::hash<Card> {
  std::size_t operator()(const Card &card) const {
    return std::hash<Card::Value>()(card.GetValue())
           ^ (std::hash<Card::Color>()(card.GetColor()) << 1);
  }
};

#endif  // CARD_H

