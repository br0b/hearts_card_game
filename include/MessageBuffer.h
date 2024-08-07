#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <deque>
#include <vector>

#include "MaybeError.h"

class MessageBuffer {
public:
  MessageBuffer(int fd, std::vector<char> &buf, const std::string &separator);

  // Modify

  // Reading from a closed socket is allowed only once.
  [[nodiscard]] MaybeError Receive();
  // Writing to a closed socket is allowed only once.
  [[nodiscard]] MaybeError Send();

  void PushMessage(const std::string &msg);
  [[nodiscard]] MaybeError PopMessage(std::string &msg);

  void ClearIncoming();

  // Query

  [[nodiscard]] bool IsEmpty() const;
  [[nodiscard]] bool IsOpen() const;
  [[nodiscard]] bool ContainsMessage() const;

private:
  // Length without seperator.
  [[nodiscard]] std::optional<size_t> GetFirstMsgLength() const;
  [[nodiscard]] MaybeError AssertIsOpen() const;

  int fd;
  // Always ends with a null byte to make separator search easier.
  std::deque<char> incoming;
  std::deque<char> outgoing;
  std::vector<char> &buffer;
  const std::string separator;
  bool isOpen;
};

#endif  // MESSAGE_H
