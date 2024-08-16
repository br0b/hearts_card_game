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
  [[nodiscard]] std::string Str() const override;

 private:
  MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  TrickNumber trickNumber;
  Hand cards;
};

#endif  // MESSAGETRICK_H

