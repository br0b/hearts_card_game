#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>

#include "MaybeError.h"
#include "MessageBuffer.h"
#include "Utilities.h"

MessageBuffer::MessageBuffer(std::array<char, 4096> &buf) : buffer(buf) {}

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

  if (fcntl(fd_, F_SETFD, O_NONBLOCK) != 0) {
    return Error::FromErrno("ConnectionStore::UpdateListening");
  }
  return std::nullopt;
}

void MessageBuffer::SetSeperator(std::string separator_) {
  separator = separator_;
}

void MessageBuffer::SetPipe(int fd_) {
  fd = fd_;
  isOpen = true;
  isReportingOn = false;
}

MaybeError MessageBuffer::Receive() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Receive",
                                   error.value()->GetMessage());
  }

  int ret = read(fd.value(), buffer.data(), sizeof(buffer));
  
  if (ret == -1) {
    if (errno == EAGAIN) {
      return std::nullopt;
    } else if (errno == ECONNRESET) {
      isOpen = false;
      return std::nullopt;
    }
    return Error::FromErrno("read");
  }

  if (ret == 0) {
    // Disconnect
    isOpen = false;
    return std::nullopt;
  }

  // Keep the null byte.
  incoming.insert(incoming.end() - 1, buffer.begin(), buffer.begin() + ret);

  return std::nullopt;
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

MaybeError MessageBuffer::Send() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Send",
                                   error.value()->GetMessage());
  }

  size_t len = std::min(outgoing.size(), buffer.size());
  std::copy_n(outgoing.begin(), len, buffer.begin());
  int ret = write(fd.value(), buffer.data(), len);

  if (ret == -1) {
    if (errno == EAGAIN) {
      return std::nullopt;
    } else if (errno == EPIPE || errno == ECONNRESET) {
      // Disconnect
      isOpen = false;
      return std::nullopt;
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
  if (isReportingOn) {
    ReportSent(msgSep);
  }
}

void MessageBuffer::ClearIncoming() {
  incoming.erase(incoming.begin(), incoming.end() - 1);
}

void MessageBuffer::DisableReporting() {
  isReportingOn = false;
}

bool MessageBuffer::IsOpen() const {
  return isOpen;
}

bool MessageBuffer::IsOutgoingEmpty() const {
  return outgoing.empty();
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

