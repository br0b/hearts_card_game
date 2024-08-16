#ifndef TRICK_NUMBER_H
#define TRICK_NUMBER_H

#include "MaybeError.h"

class TrickNumber {
 public:
  TrickNumber();

  [[nodiscard]] MaybeError Parse(std::string str);
  [[nodiscard]] MaybeError Set(int number_);
  
  int Get() const;

  friend std::ostream& operator<<(std::ostream &os, const TrickNumber &x);
  friend bool operator==(const TrickNumber &l, const TrickNumber &r);

 private:
  int number;

  constexpr static int lowerBound = 1;
  constexpr static int upperBound = 13;
};

#endif  // TRICK_NUMBER_H

