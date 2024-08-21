#ifndef MESSAGE_POINTS_H
#define MESSAGE_POINTS_H

#include <array>
#include <string>

#include "Message.h"

class MessagePoints final : public Message {
 public:
  MessagePoints(std::string header);

  void SetHeader(std::string header_);
  void SetPoints(std::array<int, 4> points);

  [[nodiscard]] const std::array<int, 4> &GetPoints() const;
  [[nodiscard]] std::string Str() const override;
  [[nodiscard]] std::optional<std::string> UserStr() const override;

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  std::string header;
  std::array<int, 4> points;
};

#endif  // MESSAGE_POINTS_H

