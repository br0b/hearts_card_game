#ifndef MESSAGE_WRONG_H
#define MESSAGE_WRONG_H

#include "Message.h"
#include "TrickNumber.h"

class MessageWrong final : public Message {
 public:
  void SetTrickNumber(TrickNumber trickNumber_);

  [[nodiscard]] TrickNumber GetTrickNumber() const;
  [[nodiscard]] std::string Str() const override;
  [[nodiscard]] std::optional<std::string> UserStr() const override;

 private:
  MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  TrickNumber trickNumber;
};

#endif  // MESSAGE_WRONG_H

