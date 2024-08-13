#ifndef MESSAGE_POINTS_H
#define MESSAGE_POINTS_H

#include <array>
#include <string>

#include "Message.h"

class MessagePoints final : public Message {
 public:
  MessagePoints(std::string header);

  void SetPoints(std::array<int, 4> points);

  [[nodiscard]] const std::array<int, 4> &GetPoints() const;

  friend std::ostream &operator<<(std::ostream &os, const MessagePoints &msg);

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  std::string header;
  std::array<int, 4> points;
};

#endif  // MESSAGE_POINTS_H

