#ifndef MESSAGETRICK_H
#define MESSAGETRICK_H

#include "Hand.h"
#include "Message.h"
#include "TrickNumber.h"

class MessageTrick final : public Message {
 public:
  void SetTrickNumber(TrickNumber trickNumber_);
  void SetCards(Hand cards_);

  [[nodiscard]] TrickNumber GetTrickNumber() const;
  [[nodiscard]] const Hand &GetCards() const;

  friend std::ostream &operator<<(std::ostream &os, const MessageTrick &msg);

 private:
  MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  TrickNumber trickNumber;
  Hand cards;
};

#endif  // MESSAGETRICK_H

