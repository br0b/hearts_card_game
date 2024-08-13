#include "Message.h"
#include "MessageBusy.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessagePoints.h"
#include "MessageTaken.h"
#include "MessageTrick.h"
#include "MessageWrong.h"

std::optional<Message> Message::Deserialize(std::string str) {
  std::optional<Message> msg;

  if (str.compare(0, 4, "BUSY") == 0) {
    msg = MessageBusy();
  } else if (str.compare(0, 4, "DEAL") == 0) {
    msg = MessageDeal();
  } else if (str.compare(0, 3, "IAM") == 0) {
    msg = MessageIam();
  } else if (str.compare(0, 5, "TAKEN") == 0) {
    msg = MessageTaken();
  } else if (str.compare(0, 5, "TOTAL") == 0) {
    msg = MessagePoints("TOTAL");
  } else if (str.compare(0, 5, "TRICK") == 0) {
    msg = MessageTrick();
  } else if (str.compare(0, 5, "SCORE") == 0) {
    msg = MessagePoints("SCORE");
  } else if (str.compare(0, 5, "WRONG") == 0) {
    msg = MessageWrong();
  }

  if (msg.has_value() && msg->Parse(str).has_value()) {
    return std::nullopt;
  }

  return msg;
}

MaybeError Message::Parse(std::string str) {
  std::smatch match;
  std::regex pattern(GetPattern());

  if (std::regex_match(str, match, pattern)) {
    return SetAfterMatch(std::move(match));
  }

  return Error::InvalidArgs("Message::Parse");
}

