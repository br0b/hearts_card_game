#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>

#include "MaybeError.h"
#include "MessageBuffer.h"
#include "Utilities.h"

MessageBuffer::MessageBuffer(std::vector<char> &buf,
                             const std::string &separator)
    : incoming{'\0'}, buffer(buf), separator(separator), isOpen(false) {}

MaybeError MessageBuffer::SetSocket(int fd_) {
  MaybeError error = std::nullopt;
  std::string lAddr = "";
  std::string rAddr = "";

  if (error = Utilities::GetAddressPair(fd_, lAddr, rAddr);
      error.has_value()) {
    std::string str = error.value()->GetMessage();
    return error;
  }

  fd = fd_;
  localAddress = std::move(lAddr);
  remoteAddress = std::move(rAddr);
  isOpen = true;
  isReportingOn = true;
  return std::nullopt;
}

void MessageBuffer::SetPipe(int fd_) {
  fd = fd_;
  isOpen = true;
  isReportingOn = false;
}

MaybeError MessageBuffer::Receive(Result &res) {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Receive",
                                   error.value()->GetMessage());
  }

  int ret = read(fd.value(), buffer.data(), sizeof(buffer));
  
  if (ret == -1) {
    return Error::FromErrno("read");
  }

  if (ret == 0) {
    // Disconnect
    isOpen = false;
    res.SetClosed();
    return std::nullopt;
  }

  // Keep the null byte.
  incoming.insert(incoming.end() - 1, buffer.begin(), buffer.begin() + ret);

  res.SetOpen(PopMessage());
  return std::nullopt;
}

MaybeError MessageBuffer::Send(Result &res) {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Send",
                                   error.value()->GetMessage());
  }

  size_t len = std::min(outgoing.size(), buffer.size());
  buffer.clear();
  buffer.insert(buffer.begin(), outgoing.begin(), outgoing.begin() + len);

  int ret = write(fd.value(), buffer.data(), len);

  if (ret == -1) {
    if (errno == EPIPE) {
      // Disconnect
      isOpen = false;
      res.SetClosed();
      return std::nullopt;
    } else {
      return Error::FromErrno("write");
    }
  }

  outgoing.erase(outgoing.begin(), outgoing.begin() + ret);

  res.SetOpen(std::nullopt);
  return std::nullopt;
}

void MessageBuffer::PushMessage(const std::string &msg) {
  std::string msgSep = msg + separator;
  outgoing.insert(outgoing.end(), msgSep.begin(), msgSep.end());
  if (isReportingOn) {
    ReportSent(msgSep);
  }
}

std::optional<std::string> MessageBuffer::PopMessage() {
  auto len = GetFirstMsgLength();
  
  if (!len.has_value()) {
    return std::nullopt;
  }

  std::string msg;
  msg.insert(msg.begin(), incoming.begin(), incoming.begin() + len.value());
  incoming.erase(incoming.begin(),
                 incoming.begin() + len.value() + separator.size());
  if (isReportingOn) {
    ReportReceived(msg + separator);
  }
  return msg;
}

void MessageBuffer::ClearIncoming() {
  incoming.erase(incoming.begin(), incoming.end() - 1);
}

bool MessageBuffer::IsEmpty() const {
  return (incoming.size() == 1) && outgoing.empty();
}

const std::optional<std::string> &MessageBuffer::GetRemote() const {
  return remoteAddress;
}

void MessageBuffer::ReportReceived(const std::string &msg) {
  ReportMessage(msg, remoteAddress.value(), localAddress.value());
}

void MessageBuffer::ReportSent(const std::string &msg) {
  ReportMessage(msg, localAddress.value(), remoteAddress.value());
}

void MessageBuffer::ReportMessage(const std::string &msg,
                                  const std::string &srcAddr,
                                  const std::string &dstAddr) {
  std::ostringstream oss;
  oss << "[" << srcAddr << "," << dstAddr << ","
      << Utilities::GetTimeStr() << "] " << msg;
  std::cout << oss.str();
}

std::optional<size_t> MessageBuffer::GetFirstMsgLength() const {
  const char *cFind = strstr(&incoming[0], separator.c_str());
  if (cFind == NULL) {
    return std::nullopt;
  }
  
  return static_cast<size_t>(cFind - &incoming[0]);
}

MaybeError MessageBuffer::AssertIsOpen() const {
  if (!isOpen) {
    return std::make_unique<Error>("MessageBuffer::AssertIsOpen",
                                   "The buffer is closed.");
  }
  return std::nullopt;
}

void MessageBuffer::Result::SetOpen(std::optional<std::string> message_) {
  message = std::move(message_);
  isClosed = false;
}

void MessageBuffer::Result::SetClosed() {
  message.reset();
  isClosed = true;
}

MaybeError MessageBuffer::Result::GetMessage(std::optional<std::string> &message_) const {
  if (isClosed) {
    return std::make_unique<Error>("MessageBuffer::Result::GetMessage",
                                   "The buffer is closed.");
  }

  message_ = message;
  return std::nullopt;
}

bool MessageBuffer::Result::IsClosed() const {
  return isClosed;
}

