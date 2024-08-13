#ifndef MESSAGE_TAKEN_H
#define MESSAGE_TAKEN_H

#include "Hand.h"
#include "Message.h"
#include "Seat.h"
#include "TrickNumber.h"
#include "TrickNumber.h"

class MessageTaken final : public Message {
 public:
  void SetTrickNumber(TrickNumber trickNumber_);
  void SetCards(Hand cards_);
  void SetTaker(Seat taker_);

  [[nodiscard]] TrickNumber GetTrickNumber() const;
  [[nodiscard]] const Hand& GetCards() const;
  [[nodiscard]] Seat GetTaker() const;

  friend std::ostream &operator<<(std::ostream &os, const MessageTaken &msg);

 private:
  MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  TrickNumber trickNumber;
  Hand cards;
  Seat taker;
};

#endif  // MESSAGE_TAKEN_H

