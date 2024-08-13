#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "DealConfig.h"
#include "MaybeError.h"

class GameConfig {
 public:
  // If the file exists, then assumes it has the right format.
  [[nodiscard]] MaybeError Set(std::string fileName);

  [[nodiscard]] const std::vector<DealConfig>& Get() const;

 private:
  std::vector<DealConfig> deals;
};

#endif  // GAME_CONFIG_H

