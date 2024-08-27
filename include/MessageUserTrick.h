#ifndef MESSAGE_PLAY_TRICK_H
#define MESSAGE_PLAY_TRICK_H

#include "Card.h"
#include "Message.h"

class MessageUserTrick final : public Message {
 public:
  void SetCard(Card card_);

  Card GetCard() const;
  [[nodiscard]] std::string Str() const override;
  [[nodiscard]] std::optional<std::string> UserStr() const override;

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  Card card;
};

#endif  // MESSAGE_PLAY_TRICK_H

