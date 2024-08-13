#ifndef MESSAGE_WRONG_H
#define MESSAGE_WRONG_H

#include "Message.h"
#include "TrickNumber.h"

class MessageWrong final : public Message {
 public:
  void SetTrickNumber(TrickNumber trickNumber_);

  [[nodiscard]] TrickNumber GetTrickNumber() const;

  friend std::ostream &operator<<(std::ostream &os, const MessageWrong &msg);

 private:
  MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  TrickNumber trickNumber;
};

#endif  // MESSAGE_WRONG_H

