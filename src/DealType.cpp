//
// Created by robert-grigoryan on 5/30/24.
//

#include "DealType.h"

std::unordered_map<char, DealType::Type> DealType::charToType = {
  {'1', Type::kTricksBad},       {'2', Type::kHeartsBad},
  {'3', Type::kQueensBad},       {'4', Type::kGentlemenBad},
  {'5', Type::kKingOfHeartsBad}, {'6', Type::kSeventhAndLastTrickBad},
  {'7', Type::kRobber}};

std::unordered_map<DealType::Type, char> DealType::typeToChar = {
  {Type::kTricksBad, '1'},       {Type::kHeartsBad, '2'},
  {Type::kQueensBad, '3'},       {Type::kGentlemenBad, '4'},
  {Type::kKingOfHeartsBad, '5'}, {Type::kSeventhAndLastTrickBad, '6'},
  {Type::kRobber, '7'}};

std::optional<DealType> DealType::GameTypeFromChar(const char _type) {
  if (charToType.contains(_type)) {
    return DealType(charToType[_type]);
  }

  return std::nullopt;
}

std::vector<DealType> DealType::getAllTypes() {
  return {
    DealType(Type::kTricksBad),       DealType(Type::kHeartsBad),
    DealType(Type::kQueensBad),       DealType(Type::kGentlemenBad),
    DealType(Type::kKingOfHeartsBad), DealType(Type::kSeventhAndLastTrickBad),
    DealType(Type::kRobber)};
}

DealType::Type DealType::getType() const { return type; }

char DealType::serialize() const { return typeToChar[type]; }

std::ostream& operator<<(std::ostream& os, const DealType& gameType) {
  os << "GameType{type=" << gameType.serialize() << "}";
  return os;
}
