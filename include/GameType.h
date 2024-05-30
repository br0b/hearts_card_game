//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef GAMETYPE_H
#define GAMETYPE_H

#include <optional>
#include <ostream>
#include <unordered_map>

class GameType {
 public:
  enum class Type {
    kTricksBad = 1,
    kHeartsBad = 2,
    kQueensBad = 3,
    kGentlemenBad = 4,
    kKingOfHeartsBad = 5,
    kSeventhAndLastTrickBad = 6,
    kRobber = 7
  };

  explicit GameType(const Type _type) : type(_type) {}
  static std::optional<GameType> GameTypeFromChar(char _type);

  [[nodiscard]] Type getType() const;
  [[nodiscard]] char getTypeAsChar() const;

 private:
  Type type;

  static std::unordered_map<char, Type> charToType;
  static std::unordered_map<Type, char> typeToChar;
};

std::ostream& operator<<(std::ostream& os, const GameType& gameType);

#endif //GAMETYPE_H
