#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <string>
#include <deque>
#include <vector>

#include "MaybeError.h"

class MessageBuffer {
 public:
  class Result {
   public:
    void SetOpen(std::optional<std::string> message_);
    void SetClosed();

    // Assumes isClosed == false.
    MaybeError GetMessage(std::optional<std::string> &message_) const;
    bool IsClosed() const;

   private:
    std::optional<std::string> message;
    bool isClosed;
  };

  MessageBuffer(std::vector<char> &buf);

  // Modify

  [[nodiscard]] MaybeError SetSocket(int fd_);
  void SetPipe(int fd_);
  void SetSeperator(std::string separator_);

  // Reading from a closed socket is allowed only once.
  [[nodiscard]] MaybeError Receive(Result &res);
  // Writing to a closed socket is allowed only once.
  [[nodiscard]] MaybeError Send(Result &res);
  void PushMessage(const std::string &msg);

  void ClearIncoming();

  // Query

  [[nodiscard]] bool IsEmpty() const;
  [[nodiscard]] const std::optional<std::string> &GetRemote() const;

 private:
  [[nodiscard]] std::optional<std::string> PopMessage();

  // Argument msg ending with a seperator.
  void ReportReceived(const std::string &msg);
  // Argument msg ending with a seperator.
  void ReportSent(const std::string &msg);
  // Argument msg ending with a seperator.
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
  std::string separator = "\r\n";
  bool isOpen;
  bool isReportingOn;
};

#endif  // MESSAGE_BUFFER_H
