//
// Created by robert-grigoryan on 5/30/24.
//

#include "GameType.h"

std::unordered_map<char, GameType::Type> GameType::charToType = {
  {'1', Type::kTricksBad},       {'2', Type::kHeartsBad},
  {'3', Type::kQueensBad},       {'4', Type::kGentlemenBad},
  {'5', Type::kKingOfHeartsBad}, {'6', Type::kSeventhAndLastTrickBad},
  {'7', Type::kRobber}};

std::unordered_map<GameType::Type, char> GameType::typeToChar = {
  {Type::kTricksBad, '1'},       {Type::kHeartsBad, '2'},
  {Type::kQueensBad, '3'},       {Type::kGentlemenBad, '4'},
  {Type::kKingOfHeartsBad, '5'}, {Type::kSeventhAndLastTrickBad, '6'},
  {Type::kRobber, '7'}};

std::optional<GameType> GameType::GameTypeFromChar(const char _type) {
  if (charToType.contains(_type)) {
    return GameType(charToType[_type]);
  }

  return std::nullopt;
}

GameType::Type GameType::getType() const { return type; }

char GameType::getTypeAsChar() const { return typeToChar[type]; }

std::ostream& operator<<(std::ostream& os, const GameType& gameType) {
  os << "GameType{type=" << gameType.getTypeAsChar() << "}";
  return os;
}
