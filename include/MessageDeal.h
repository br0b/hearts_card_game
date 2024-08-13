#ifndef MESSAGEDEAL_H
#define MESSAGEDEAL_H

#include "DealType.h"
#include "Hand.h"
#include "Message.h"
#include "Seat.h"

class MessageDeal final : public Message {
 public:
  void SetType(DealType type_);
  void SetFirst(Seat first_);
  void SetHand(Hand hand_);

  DealType GetType() const;
  Seat GetFirst() const;
  const Hand& GetHand() const;

  friend std::ostream& operator<<(std::ostream &os, const MessageDeal &msg);

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  DealType type;
  Seat first;
  Hand hand;
};

#endif  // MESSAGEDEAL_H
