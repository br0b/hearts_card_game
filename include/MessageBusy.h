#ifndef MESSAGEBUSY_H
#define MESSAGEBUSY_H

#include <vector>

#include "MaybeError.h"
#include "Message.h"
#include "Seat.h"

class MessageBusy final : public Message {
 public:
  void SetSeats(std::vector<Seat> seats_);

  [[nodiscard]] const std::vector<Seat>& GetSeats() const;
  [[nodiscard]] std::string Str() const override;

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() const override;

  std::vector<Seat> seats;
};

#endif  // MESSAGEBUSY_H

