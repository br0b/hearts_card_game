#include "Card.h"

Card::Card(Value value, Color color) : value(value), color(color) {}

std::optional<Card::Value> Card::ParseValue(std::string value) {
  if (value.size() > 2) {
    return std::nullopt;
  } else if (value.size() == 2) {
    if (value.compare(0, 2, "10") == 0) {
      return Value::k10;
    } else {
      return std::nullopt;
    }
  }

  switch (value[0]) {
    case '2': {
      return Card::Value::k2;
    }
    case '3': {
      return Card::Value::k3;
    }
    case '4': {
      return Card::Value::k4;
    }
    case '5': {
      return Card::Value::k5;
    }
    case '6': {
      return Card::Value::k6;
    }
    case '7': {
      return Card::Value::k7;
    }
    case '8': {
      return Card::Value::k8;
    }
    case '9': {
      return Card::Value::k9;
    }
    case 'J': {
      return Card::Value::kJ;
    }
    case 'Q': {
      return Card::Value::kQ;
    }
    case 'K': {
      return Card::Value::kK;
    }
    case 'A': {
      return Card::Value::kA;
    }
    default: {
      return std::nullopt;
    }
  };
}

std::optional<Card::Color> Card::ParseColor(std::string color) {
  if (color.size() > 1) {
    return std::nullopt;
  }

  switch (color[0]) {
    case 'C': {
      return Card::Color::kCub;
    }
    case 'D': {
      return Card::Color::kDiamond;
    }
    case 'H': {
      return Card::Color::kHeart;
    }
    case 'S': {
      return Card::Color::kSpade;
    }
    default: {
      return std::nullopt;
    }
  }
}

Card::Value Card::GetValue() const {
  return value;
}

Card::Color Card::GetColor() const {
  return color;
}

int Card::GetColorIndex() const {
  return static_cast<int>(color);
}

bool Card::operator==(const Card &other) const {
  return value == other.value && color == other.color;
}

std::ostream& operator<<(std::ostream &os, const Card &c) {
  switch (c.value) {
    case Card::Value::k2: {
      os << '2';
      break;
    }
    case Card::Value::k3: {
      os << '3';
      break;
    }
    case Card::Value::k4: {
      os << '4';
      break;
    }
    case Card::Value::k5: {
      os << '5';
      break;
    }
    case Card::Value::k6: {
      os << '6';
      break;
    }
    case Card::Value::k7: {
      os << '7';
      break;
    }
    case Card::Value::k8: {
      os << '8';
      break;
    }
    case Card::Value::k9: {
      os << '9';
      break;
    }
    case Card::Value::k10: {
      os << "10";
      break;
    }
    case Card::Value::kJ: {
      os << 'J';
      break;
    }
    case Card::Value::kQ: {
      os << 'Q';
      break;
    }
    case Card::Value::kK: {
      os << 'K';
      break;
    }
    case Card::Value::kA: {
      os << 'A';
      break;
    }
  }

  switch (c.color) {
    case Card::Color::kCub: {
      os << 'C';
      break;
    }
    case Card::Color::kDiamond: {
      os << 'D';
      break;
    }
    case Card::Color::kHeart: {
      os << 'H';
      break;
    }
    case Card::Color::kSpade: {
      os << 'S';
      break;
    }
  }

  return os;
}

