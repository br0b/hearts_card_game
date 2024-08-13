#ifndef MESSAGE_POINTS_H
#define MESSAGE_POINTS_H

#include <string>

#include "Message.h"
#include "Points.h"

class MessagePoints final : public Message {
 public:
  MessagePoints(std::string header);

  void SetPoints(Points points);

  [[nodiscard]] const Points& GetPoints() const;

  friend std::ostream& operator<<(std::ostream &os, const MessagePoints &msg);

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  std::string header;
  Points points;
};

#endif  // MESSAGE_POINTS_H

