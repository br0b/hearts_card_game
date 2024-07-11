//
// Created by robert-grigoryan on 6/9/24.
//
#include <iomanip>
#include <utility>

#include "MessageDEAL.h"

std::unique_ptr<MessageDEAL> MessageDEAL::FromConfig(
    const DealConfig& dealConfig, Seat::Position player) {
  std::ostringstream msg;
  const DealType dealType = dealConfig.getDealType();
  const Seat::Position firstPlayer = dealConfig.getFirstPlayer();
  const HandConfig& handConfig = dealConfig.getHandConfig(player);
  msg << "DEAL" << dealType.serialize() << Seat(firstPlayer).serialize() << handConfig.serialize()
      << getSeperator();

  return std::unique_ptr<MessageDEAL>(
      new MessageDEAL(msg.str(), dealType, firstPlayer, handConfig));
}

MessageDEAL::MessageDEAL(const std::string& _message, const DealType _dealType,
                         const Seat::Position _firstPlayer,
                         HandConfig _handConfig)
    : Message(_message),
      dealType(_dealType),
      firstPlayer(_firstPlayer),
      handConfig(std::move(_handConfig)) {}
