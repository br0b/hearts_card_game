#ifndef MESSAGEIAM_H
#define MESSAGEIAM_H

#include "Message.h"
#include "Seat.h"

class MessageIam final : public Message {
 public:
  void SetSeat(Seat seat_);

  [[nodiscard]] Seat GetSeat() const;

  friend std::ostream& operator<<(std::ostream &os, const MessageIam &msg);

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  Seat seat;
};

#endif  // MESSAGEIAM_H
