#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include <regex>
#include <string>

#include "MaybeError.h"

class Message {
 public:
  virtual ~Message();
  // Returns null if str is not a valid message.
  [[nodiscard]] static std::unique_ptr<Message> Deserialize(std::string str);
  friend std::ostream &operator<<(std::ostream &os, const Message &msg);
  [[nodiscard]] MaybeError GetMaybeUserStr(std::string &userStr);
  [[nodiscard]] virtual std::string Str() const = 0;
  [[nodiscard]] virtual std::optional<std::string> UserStr() const = 0;
 
 private:
  [[nodiscard]] MaybeError Parse(std::string str);
  [[nodiscard]] virtual MaybeError SetAfterMatch(std::smatch match) = 0;
  [[nodiscard]] virtual std::string GetPattern() const = 0;
};

#endif  // MESSAGE_H
