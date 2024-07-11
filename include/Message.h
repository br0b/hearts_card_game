//
// Created by robert-grigoryan on 6/6/24.
//
#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "Error.h"

class Message {
public:
  static std::variant<std::unique_ptr<Message>, Error> Create(
      const std::string& message);

  static bool isValidMessage(const std::string& message);
  static bool containsMessage(const std::string& message);
  // Returns the shortest prefix of arg message that contains a message.
  static std::optional<std::unique_ptr<Message>> getFirstMessage(
      const std::string& message);

  [[nodiscard]] virtual std::string getMessage() const;
  [[nodiscard]] virtual std::string getMessageWithoutSeperator() const;
  virtual ~Message() ;

protected:
  explicit Message(std::string _message);
  [[nodiscard]] static std::string getSeperator();

  std::string message;
};

#endif  // MESSAGE_H
