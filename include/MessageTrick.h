//
// Created by robert-grigoryan on 6/9/24.
//
#ifndef MESSAGETRICK_H
#define MESSAGETRICK_H

#include <regex>

#include "Message.h"
#include "Trick.h"

class MessageTrick final : public Message {
public:
  static std::unique_ptr<MessageTrick> Create(int trickNumber,
                                              const std::vector<Card>& trick);
  static std::variant<std::unique_ptr<MessageTrick>, Error> FromPlayerMessage(
      const Message& msgTrick);

  [[nodiscard]] int getTrickNumber() const;
  [[nodiscard]] std::vector<Card> getCards() const;

private:
  int trickNumber;
  std::vector<Card> cards;
  static const std::regex pattern;

  MessageTrick(const std::string& _message, int _trickNumber,
               std::vector<Card> _cards);
};

#endif  // MESSAGETRICK_H
