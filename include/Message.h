#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include <optional>
#include <regex>
#include <string>

#include "MaybeError.h"

class Message {
 public:
  [[nodiscard]] static std::optional<std::unique_ptr<Message>> Deserialize(
      std::string str);
 
 private:
  [[nodiscard]] MaybeError Parse(std::string str);
  [[nodiscard]] virtual MaybeError SetAfterMatch(std::smatch match) = 0;
  [[nodiscard]] virtual std::string GetPattern() = 0;
};

#endif  // MESSAGE_H
