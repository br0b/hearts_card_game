//
// Created by robert-grigoryan on 6/6/24.
//
#include <memory>

#include "Logger.h"
#include "Message.h"


std::variant<std::unique_ptr<Message>, Error> Message::Create(
    const std::string& message) {
  if (isValidMessage(message)) {
    return std::unique_ptr<Message>(new Message(message));
  }

  return Error("ERROR: Invalid message: " + message);
}

bool Message::isValidMessage(const std::string& message) {
  // Check if string ends with \r\n.
  if (message.size() < 2) {
    return false;
  }
  if (message[message.size() - 2] != '\r' ||
      message[message.size() - 1] != '\n') {
    return false;
  }

  return true;
}

bool Message::containsMessage(const std::string& message) {
  return message.find("\r\n") != std::string::npos;
}

std::optional<std::unique_ptr<Message>> Message::getFirstMessage(
    const std::string& message) {
  if (!containsMessage(message)) {
    return std::nullopt;
  }

  return std::unique_ptr<Message>(
      new Message(message.substr(0, message.find("\r\n") + 2)));
}

std::string Message::getMessage() const { return message; }

std::string Message::getMessageWithoutSeperator() const {
  return message.substr(0, message.size() - getSeperator().size());
}

Message::~Message() = default;

std::string Message::getSeperator() { return "\r\n"; }

Message::Message(std::string _message) : message(std::move(_message)) {}
