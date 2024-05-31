//
// Created by robert-grigoryan on 5/30/24.
//

#ifndef GAMETYPE_H
#define GAMETYPE_H

#include <optional>
#include <ostream>
#include <unordered_map>
#include <vector>

class DealType {
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

  explicit DealType(const Type _type) : type(_type) {}
  static std::optional<DealType> GameTypeFromChar(char _type);
  static std::vector<DealType> getAllTypes();

  [[nodiscard]] Type getType() const;
  [[nodiscard]] char serialize() const;

 private:
  Type type;

  static std::unordered_map<char, Type> charToType;
  static std::unordered_map<Type, char> typeToChar;
};

std::ostream& operator<<(std::ostream& os, const DealType& gameType);

#endif //GAMETYPE_H
