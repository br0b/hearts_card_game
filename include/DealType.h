#ifndef TYPE_H
#define TYPE_H

#include <iostream>

#include "MaybeError.h"

class DealType {
 public:
  enum class Value {
    kTricksBad = 0,
    kHeartsBad,
    kQueensBad,
    kGentlemenBad,
    kKingOfHeartsBad,
    kSeventhAndLastTrickBad,
    kRobber,
  };

  [[nodiscard]] MaybeError Parse(std::string str);

  [[nodiscard]] Value Get() const;
  friend std::ostream& operator<<(std::ostream &os, const DealType &dt);

 private:
  Value value;
};

#endif  // TYPE_H
