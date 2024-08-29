#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <array>
#include <deque>
#include <string>

#include "MaybeError.h"

class MessageBuffer {
 public:
  MessageBuffer(std::array<char, 4096> &buf);

  // Modify

  [[nodiscard]] MaybeError SetSocket(int fd_);
  void SetPipe(int fd_);
  void SetSeperator(std::string separator_);

  // Reading from a closed socket is allowed only once.
  [[nodiscard]] MaybeError Receive();
  std::optional<std::string> PopMessage();
  // Writing to a closed socket is allowed only once.
  [[nodiscard]] MaybeError Send();
  void PushMessage(const std::string &msg);

  void ClearIncoming();

  void DisableReporting();

  // Query

  [[nodiscard]] bool IsOpen() const;
  [[nodiscard]] bool IsOutgoingEmpty() const;
  [[nodiscard]] const std::optional<std::string> &GetRemote() const;

 private:

  // Argument msg ending with a seperator.
  void ReportReceived(const std::string &msg);
  // Argument msg ending with a seperator.
  void ReportSent(const std::string &msg);
  // Argument msg ending with a seperator.
  void ReportMessage(const std::string &msg, const std::string &srcAddr,
                     const std::string &dstAddr);

  [[nodiscard]] MaybeError AssertIsOpen() const;

  std::optional<int> fd;
  std::optional<std::string> localAddress;
  std::optional<std::string> remoteAddress;
  std::deque<char> incoming;
  std::deque<char> outgoing;
  std::array<char, 4096> &buffer;
  std::string separator = "\r\n";
  bool isOpen = false;
  bool isReportingOn = true;
};

#endif  // MESSAGE_BUFFER_H
