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

 private:
  int number;
};

#endif  // TRICK_NUMBER_H

