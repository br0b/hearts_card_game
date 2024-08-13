#include "GameConfig.h"
#include <memory>
#include "MaybeError.h"

MaybeError GameConfig::Set(const std::vector<std::string> &lines) {
  auto it = lines.begin();
  std::string header = "";
  std::vector<std::string> hands;
  auto error = std::make_unique<Error>("GameConfig::Set", "Invalid argument.");
  MaybeError ret = std::nullopt;

  if (lines.size() % 5 != 0) {
    return error;
  }

  deals.clear();
  while (it != lines.end()) {
    deals.emplace_back();
    if (it->size() != 2) {
      return error;
    }
    if (ret = deals.back().SetType((*it)[0]); ret.has_value()) {
      return ret;
    }
    if (ret = deals.back().SetSeat((*it)[1]); ret.has_value()) {
      return ret;
    }
    if (ret = deals.back().SetHands(std::vector<std::string>(it + 1, it + 4));
        ret.has_value()) {
      return ret;
    }
    it += 5;
  }
  return std::nullopt;
}

const std::vector<DealConfig>& GameConfig::Get() const {
  return deals;
}

