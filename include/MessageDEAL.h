//
// Created by robert-grigoryan on 6/9/24.
//
#ifndef MESSAGEDEAL_H
#define MESSAGEDEAL_H

#include "DealConfig.h"
#include "DealType.h"
#include "HandConfig.h"
#include "Message.h"
#include "Seat.h"

class MessageDEAL final : public Message {
public:
  static std::unique_ptr<MessageDEAL> FromConfig(const DealConfig& dealConfig,
                                                 Seat::Position player);

private:
  MessageDEAL(const std::string& _message, DealType _dealType,
              Seat::Position _firstPlayer, HandConfig _handConfig);

  DealType dealType;
  Seat::Position firstPlayer;
  HandConfig handConfig;
};

#endif  // MESSAGEDEAL_H
