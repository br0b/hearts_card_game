//
// Created by robert-grigoryan on 6/9/24.
//
#include <utility>

#include "MessageTrick.h"

std::unique_ptr<MessageTrick> MessageTrick::Create(
    const int trickNumber, const std::vector<Card>& _cards) {
  std::ostringstream msg;
  msg << "TRICK" << std::to_string(trickNumber);
  for (Card card : _cards) {
    msg << card;
  }
  msg << getSeperator();
  return std::unique_ptr<MessageTrick>(
      new MessageTrick(msg.str(), trickNumber, _cards));
}

std::variant<std::unique_ptr<MessageTrick>, Error>
MessageTrick::FromPlayerMessage(const Message& msgTrick) {
  std::string msg = msgTrick.getMessage();
  Error err("ERROR: Invalid message: " + msgTrick.getMessageWithoutSeperator());
  std::smatch matches;
  if (std::regex_search(msg, matches, pattern) == false) {
    return err;
  }
  int trickNumber = std::stoi(matches[1].str());
  auto card = Card::FromString(matches[2].str());
  if (std::holds_alternative<Error>(card)) {
    return std::get<Error>(card);
  }

  return std::unique_ptr<MessageTrick>(
      new MessageTrick(msg, trickNumber, std::vector{std::get<Card>(card)}));
}

int MessageTrick::getTrickNumber() const { return trickNumber; }

std::vector<Card> MessageTrick::getCards() const { return cards; }

MessageTrick::MessageTrick(const std::string& _message, const int _trickNumber,
                           std::vector<Card> _cards)
    : Message(_message), trickNumber(_trickNumber), cards(std::move(_cards)) {}

const std::regex MessageTrick::pattern(
    "\bTRICK(1[0-3]|[1-9])(10|[2-9]|[JQKA])[CDHS]\\r\\n\b");
