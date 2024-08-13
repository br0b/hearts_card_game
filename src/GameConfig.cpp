#include <fstream>
#include <memory>
#include <string>

#include "DealConfig.h"
#include "GameConfig.h"
#include "MaybeError.h"

MaybeError GameConfig::Set(std::string fileName) {
  std::array<Hand, 4> hands;
  DealType type;
  Seat first;
  std::string line;
  std::unique_ptr<Error> error;

  std::ifstream file(fileName);

  if (!file.is_open()) {
    return std::make_unique<Error>("GameConfig::Set",
                                   "Unable to open the file.");
  }

  deals.clear();
  while (std::getline(file, line)) {
    MaybeError ret;
    deals.emplace_back();

    if (ret = type.Parse(std::string{line[0]}); ret.has_value()) {
      file.close();
      error = std::move(ret.value());
      break;
    }

    if (ret = first.Parse(std::string{line[1]}); ret.has_value()) {
      error = std::move(ret.value());
      break;
    }
    
    for (Hand &hand : hands) {
      std::getline(file, line);
      hand.Parse(line);
    }

    deals.back().SetType(type);
    deals.back().SetFirst(first);
    deals.back().SetHands(hands);
  }

  file.close();
  if (error.get() != nullptr) {
    return error;
  }

  return std::nullopt;
}

const std::vector<DealConfig>& GameConfig::Get() const {
  return deals;
}

