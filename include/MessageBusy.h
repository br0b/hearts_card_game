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

  friend std::ostream& operator<<(std::ostream &os, const MessageBusy &msg);

 private:
  [[nodiscard]] MaybeError SetAfterMatch(std::smatch match) override;
  [[nodiscard]] std::string GetPattern() override;

  std::vector<Seat> seats;
};

#endif  // MESSAGEBUSY_H

