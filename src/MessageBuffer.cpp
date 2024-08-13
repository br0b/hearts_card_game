#include <memory>
#include <sstream>
#include "Logger.h"
#include "MaybeError.h"
#include "Utilities.h"
#include <string.h>

#include "MessageBuffer.h"

MessageBuffer::MessageBuffer(std::vector<char> &buf,
                             const std::string &separator)
    : incoming{'\0'}, buffer(buf), separator(separator), isOpen(false) {}

MaybeError MessageBuffer::SetSocket(int fd_) {
  MaybeError error = std::nullopt;
  std::string lAddr = "";
  std::string rAddr = "";

  if (error = Utilities::GetAddressPair(fd_, lAddr, rAddr);
      error.has_value()) {
    return error;
  }

  fd = fd_;
  localAddress = std::move(lAddr);
  remoteAddress = std::move(rAddr);
  isOpen = true;
  isLoggingOn = true;
  return std::nullopt;
}

void MessageBuffer::SetPipe(int fd_) {
  fd = fd_;
  isOpen = true;
  isLoggingOn = false;
}

MaybeError MessageBuffer::Receive() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Receive",
                                   error.value()->GetMessage());
  }

  int ret = read(fd.value(), &buffer[0], sizeof(buffer));
  
  if (ret == -1) {
    return Error::FromErrno("read");
  }

  if (ret == 0) {
    // Disconnect
    isOpen = false;
  }

  // Keep the null byte.
  incoming.insert(incoming.end() - 1, buffer.begin(), buffer.begin() + ret);

  return std::nullopt;
}

MaybeError MessageBuffer::Send() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Send",
                                   error.value()->GetMessage());
  }

  size_t len = std::min(outgoing.size(), buffer.size());
  int ret = write(fd.value(), &outgoing[0], len);

  if (ret == -1) {
    if (errno == EPIPE) {
      // Disconnect
      isOpen = false;
    } else {
      return Error::FromErrno("write");
    }
  }

  outgoing.erase(outgoing.begin(), outgoing.begin() + ret);

  return std::nullopt;
}

void MessageBuffer::PushMessage(const std::string &msg) {
  std::string msgSep = msg + separator;
  outgoing.insert(outgoing.end(), msgSep.begin(), msgSep.end());
  if (isLoggingOn) {
    ReportSent(msgSep);
  }
}

MaybeError MessageBuffer::PopMessage(std::string &msg) {
  msg.clear();
  auto len = GetFirstMsgLength();
  
  if (!len.has_value()) {
    return std::make_unique<Error>("MessageBuffer::PopMessage", "No message.");
  }

  msg.insert(msg.begin(), incoming.begin(), incoming.begin() + len.value());
  incoming.erase(incoming.begin(),
                 incoming.begin() + len.value() + separator.size());
  if (isLoggingOn) {
    ReportReceived(msg + separator);
  }
  return std::nullopt;
}

void MessageBuffer::ClearIncoming() {
  incoming.erase(incoming.begin(), incoming.end() - 1);
}

bool MessageBuffer::IsEmpty() const {
  return (incoming.size() == 1) && outgoing.empty();
}

bool MessageBuffer::IsOpen() const {
  return isOpen;
}

bool MessageBuffer::ContainsMessage() const {
  return GetFirstMsgLength().has_value();
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
  oss << "[" << srcAddr << "," << dstAddr << "," << Utilities::GetTimeStr() << "] " << msg;
  Logger::Report(oss.str());
}

std::optional<size_t> MessageBuffer::GetFirstMsgLength() const {
  const char *cFind = strstr(&incoming[0], separator.c_str());
  if (cFind == NULL) {
    return std::nullopt;
  }
  
  return static_cast<size_t>(cFind - &incoming[0]) + separator.size();
}

MaybeError MessageBuffer::AssertIsOpen() const {
  if (!isOpen) {
    return std::make_unique<Error>("The buffer is closed.");
  }
  return std::nullopt;
}

