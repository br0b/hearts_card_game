#include "Message.h"
#include <memory>
#include "MessageBusy.h"
#include "MessageDeal.h"
#include "MessageIam.h"
#include "MessagePlayTrick.h"
#include "MessagePoints.h"
#include "MessageTaken.h"
#include "MessageTrick.h"
#include "MessageWrong.h"

Message::~Message() {}

std::unique_ptr<Message> Message::Deserialize(std::string str) {
  std::unique_ptr<Message> msg;

  if (str.compare(0, 4, "BUSY") == 0) {
    msg = std::make_unique<MessageBusy>();
  } else if (str.compare(0, 4, "DEAL") == 0) {
    msg = std::make_unique<MessageDeal>();
  } else if (str.compare(0, 3, "IAM") == 0) {
    msg = std::make_unique<MessageIam>();
  } else if (str.compare(0, 5, "TAKEN") == 0) {
    msg = std::make_unique<MessageTaken>();
  } else if (str.compare(0, 5, "TOTAL") == 0) {
    msg = std::make_unique<MessagePoints>("TOTAL");
  } else if (str.compare(0, 5, "TRICK") == 0) {
    msg = std::make_unique<MessageTrick>();
  } else if (str.compare(0, 5, "SCORE") == 0) {
    msg = std::make_unique<MessagePoints>("SCORE");
  } else if (str.compare(0, 5, "WRONG") == 0) {
    msg = std::make_unique<MessageWrong>();
  } else if (str.compare(0, 1, "!") == 0) {
    msg = std::make_unique<MessagePlayTrick>();
  }

  if (msg.get() == nullptr || msg->Parse(str).has_value()) {
    return nullptr;
  }

  return msg;
}

std::ostream &operator<<(std::ostream &os, const Message &msg) {
  os << msg.Str();
  return os;
}

MaybeError Message::GetMaybeUserStr(std::string &userStr) {
  std::optional<std::string> tmp = UserStr();
  if (!tmp.has_value()) {
    return std::make_unique<Error>("Message::GetMaybeUserStr",
                                   "No user string.");
  }
  userStr = tmp.value();
  return std::nullopt;
}

MaybeError Message::Parse(std::string str) {
  std::smatch match;
  std::regex pattern(GetPattern());

  if (std::regex_match(str, match, pattern)) {
    return SetAfterMatch(std::move(match));
  }

  return Error::InvalidArgs("Message::Parse");
}

