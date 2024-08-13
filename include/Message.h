#ifndef MESSAGE_H
#define MESSAGE_H

#include <optional>
#include <regex>
#include <string>

#include "MaybeError.h"

class Message {
 public:
  [[nodiscard]] std::optional<Message> Deserialize(std::string str);
 
 private:
  [[nodiscard]] MaybeError Parse(std::string str);
  [[nodiscard]] virtual MaybeError SetAfterMatch(std::smatch match);
  [[nodiscard]] virtual std::string GetPattern();
};

#endif  // MESSAGE_H
