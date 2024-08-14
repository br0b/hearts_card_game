#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <string>
#include <deque>
#include <vector>

#include "MaybeError.h"

class MessageBuffer {
public:
  MessageBuffer(std::vector<char> &buf, const std::string &separator);

  // Modify

  [[nodiscard]] MaybeError SetSocket(int fd_);
  void SetPipe(int fd_);

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
  [[nodiscard]] const std::optional<std::string> &GetRemote() const;

private:
  // Argument msg without a seperator.
  void ReportReceived(const std::string &msg);
  // Argument msg without a seperator.
  void ReportSent(const std::string &msg);
  // Argument msg should end with a seperator.
  void ReportMessage(const std::string &msg, const std::string &srcAddr,
                     const std::string &dstAddr);

  // Length without seperator.
  [[nodiscard]] std::optional<size_t> GetFirstMsgLength() const;
  [[nodiscard]] MaybeError AssertIsOpen() const;

  std::optional<int> fd;
  std::optional<std::string> localAddress;
  std::optional<std::string> remoteAddress;
  // Always ends with a null byte to make separator search easier.
  std::deque<char> incoming;
  std::deque<char> outgoing;
  std::vector<char> &buffer;
  const std::string separator;
  bool isOpen;
  bool isReportingOn;
};

#endif  // MESSAGE_BUFFER_H
