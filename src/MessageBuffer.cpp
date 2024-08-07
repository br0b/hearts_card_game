#include <memory>
#include <sstream>
#include "MaybeError.h"
#include <string.h>

#include "MessageBuffer.h"

MessageBuffer::MessageBuffer(int fd, std::vector<char> &buf,
                             const std::string &separator)
  : fd(fd), incoming{'\0'}, buffer(buf), separator(separator), isOpen(true) {}

MaybeError MessageBuffer::Receive() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Receive",
                                   error.value()->GetMessage());
  }

  int ret = read(fd, &buffer[0], sizeof(buffer));
  
  if (ret == -1) {
    return Error::FromErrno("read");
  }

  if (ret == 0) {
    // Disconnect
    isOpen = false;
  }

  incoming.insert(incoming.end() - 1, buffer.begin(), buffer.begin() + ret);

  return std::nullopt;
}

MaybeError MessageBuffer::Send() {
  if (MaybeError error = AssertIsOpen(); error.has_value()) {
    return std::make_unique<Error>("MessageBuffer::Send",
                                   error.value()->GetMessage());
  }

  size_t len = std::min(outgoing.size(), buffer.size());
  int ret = write(fd, &outgoing[0], len);

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
  outgoing.insert(outgoing.end(), msg.begin(), msg.end());
}

MaybeError MessageBuffer::PopMessage(std::string &msg) {
  msg.clear();
  auto len = GetFirstMsgLength();
  
  if (!len.has_value()) {
    return std::make_unique<Error>("MessageBuffer::PopMessage", "No message.");
  }

  msg.insert(msg.begin(), incoming.begin(), incoming.begin() + len.value());
  incoming.erase(incoming.begin(), incoming.begin() + len.value());
  return std::nullopt;
}

void MessageBuffer::ClearIncoming() {
  incoming.clear();
}

bool MessageBuffer::IsEmpty() const {
  return incoming.empty() && outgoing.empty();
}

bool MessageBuffer::IsOpen() const {
  return isOpen;
}

bool MessageBuffer::ContainsMessage() const {
  return GetFirstMsgLength().has_value();
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
    std::ostringstream oss;
    oss << "Socket " << fd << " is closed.";
    return std::make_unique<Error>(oss.str());
  }
  return std::nullopt;
}

