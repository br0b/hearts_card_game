#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "DealConfig.h"
#include "MaybeError.h"

class GameConfig {
 public:
  [[nodiscard]] MaybeError Set(const std::vector<std::string> &lines);

  [[nodiscard]] const std::vector<DealConfig>& Get() const;

 private:
  std::vector<DealConfig> deals;
};

#endif  // GAME_CONFIG_H

