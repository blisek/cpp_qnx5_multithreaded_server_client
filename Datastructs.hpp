
#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "MessageType.hpp"
#include "Settings.hpp"
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef DEBUG
#define BUMP std::cout << "LINE: " << __LINE__ << std::endl;
#else
#define BUMP ;
#endif

struct Message {
  MessageType::MessageType messageType;
  int messageId;
  char message[::MESSAGE_LENGTH];

  template <typename T = char> void store(const T &elem, size_t index) {
    if (index >= length<T>())
      throw std::runtime_error("Index out of range");
    ((T *)message)[index] = elem;
  }

  template <typename T = char> T load(size_t index) {
    if (index >= length<T>())
      throw std::runtime_error("Index out of range");
    return ((T *)message)[index];
  }

  template <typename T = char> T *ptr(size_t index) {
    if (index >= length<T>())
      throw std::runtime_error("Index out of range");
    return ((T *)message) + index;
  }

  template <typename T = char> size_t length() {
    return ::MESSAGE_LENGTH / sizeof(T);
  }
};

template <typename T> void writeStruct(const T &t, const std::string &path) {
  int fd = ::open(path.c_str(), O_WRONLY | O_CREAT);
  if (fd == -1)
    throw std::runtime_error("Can't store settings.");
  int length = static_cast<int>(sizeof(T));
  if (::write(fd, (void *)&t, length) != length)
    throw std::runtime_error("Error while storing settings.");
  ::close(fd);
}

template <typename T> T loadStruct(const std::string &path) {
  T t;
  int fd = ::open(path.c_str(), O_RDONLY);
  if (fd == -1)
    throw std::runtime_error("Can't load settings.");
  size_t length = sizeof(T);
  if (::read(fd, (void *)&t, length) != length)
    throw std::runtime_error("Error while reading settings.");
  ::close(fd);
  return t;
}

struct ChannelSettings {
  pid_t parent_pid;
  int channel_id;
};

template <typename MessageFormat, char separator = ' ',
          size_t Length = MESSAGE_LENGTH / sizeof(MessageFormat)>
void writeMessage(std::ostream &os, const Message &msg) {
  os << "Message:\n\tType: " << msg.messageType << "\n\tText: ";
  MessageFormat *begin = (MessageFormat *)msg.message;
  MessageFormat *end = begin + Length;
  while (begin < end) {
    os << *begin++ << separator;
  }
  os << '\n';
}

#endif // DATASTRUCTS_HPP
