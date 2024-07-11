//
// Created by robert-grigoryan on 6/8/24.
//
#ifndef MESSAGEBUSY_H
#define MESSAGEBUSY_H

#include <vector>

#include "Message.h"
#include "Seat.h"

class MessageBUSY final : public Message {
public:
  static std::unique_ptr<Message> FromVector(
      const std::vector<Seat>& takenSeats);
  [[nodiscard]] std::string getMessage() const override;

private:
  explicit MessageBUSY(const std::string& _message,
                       const std::vector<Seat>& _takenSeats);

  std::vector<Seat> takenSeats;
};

#endif  // MESSAGEBUSY_H
