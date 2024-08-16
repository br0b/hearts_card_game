#ifndef MESSAGEIAM_H
#define MESSAGEIAM_H

#include "Message.h"
#include "Seat.h"

class MessageIam final : public Message {
 public:
  void SetSeat(Seat seat_);

  [[nodiscard]] Seat GetSeat() const;
  [[nodiscard]] std::string Str() const override;

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  Seat seat;
};

#endif  // MESSAGEIAM_H
