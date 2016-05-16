//
//  UPipe.hpp
//  lista8-proj
//
//  Created by BLISEK on 07/05/2016.
//
//

#ifndef UPipe_hpp
#define UPipe_hpp

#include "Datastructs.hpp"
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
  Prosta klasa do obslugi potokow.
*/
template <typename T, size_t Length = sizeof(T)> class Pipe {
protected:
  enum { READ = 0, WRITE = 1, PIPE_ARR_LENGTH = 2 };
  struct {
    int pipe0;
    int pipe1;
  } pipe_info;
  bool pipeActive;
  bool nonblocking;

  void cleanup() {
    if (pipeActive) {
      close(pipe_info.pipe0);
      close(pipe_info.pipe1);
      pipeActive = false;
    }
  }

public:
  Pipe(bool nblocking) : nonblocking(nblocking) {}

  void write(const T &msg) {
    if (!pipeActive)
      return;
    ::write(pipe_info.pipe1, (void *)&msg, Length);
  }

  int read(T &outMsg) const {
    if (!pipeActive)
      return -1;

    int bytesRead =
        ::read(pipe_info.pipe0, reinterpret_cast<void *>(&outMsg), Length);
    if (nonblocking && bytesRead == -1 && errno == EAGAIN) {
      return 0;
    } else if (bytesRead < 0) {
      throw std::runtime_error("Error during reading from pipe.");
    } else if (bytesRead != Length) {
      std::stringstream ss;
      ss << "Invalid bytes count read: ";
      ss << bytesRead;
      throw std::runtime_error(ss.str().c_str());
    } else {
      return bytesRead;
    }
  }

  void writeDescriptors() const {
    std::cout << "pipe0: " << pipe_info.pipe0 << "\npipe1: " << pipe_info.pipe1
              << std::endl;
  }
};

template <typename T, size_t Length = sizeof(T)>
class UPipe : public Pipe<T, Length> {
public:
  UPipe(bool nonblocking = false) : Pipe<T, Length>(nonblocking) {
    int upipe[2];
    if (pipe(upipe)) {
      throw std::runtime_error("Can't create unnamed pipe.");
    }

    if (nonblocking) {
      if (fcntl(upipe[this->READ], F_SETFL,
                fcntl(upipe[this->READ], F_GETFL, O_NONBLOCK) | O_NONBLOCK) ==
          -1)
        throw std::runtime_error("Can't establish nonblocking anonymous pipe.");
    }

    this->pipe_info.pipe0 = upipe[this->READ];
    this->pipe_info.pipe1 = upipe[this->WRITE];
    this->pipeActive = true;
  }

  ~UPipe() { this->cleanup(); }
};

template <typename T, size_t Length = sizeof(T)>
class NPipe : public Pipe<T, Length> {
  bool readMode, writeMode;
  const std::string pipePath;

public:
  NPipe(const std::string &path, bool wMode = true, bool rMode = true,
        bool nonblocking = false)
      : Pipe<T, Length>(nonblocking), readMode(rMode), writeMode(wMode),
        pipePath(path) {
    BUMP;
    if (wMode) {
      int status = ::mkfifo(path.c_str(), 0666);
      if (status == -1)
        throw std::runtime_error("Can't create named pipe.");
      int writePipe = ::open(path.c_str(), O_WRONLY);
      BUMP;
      if (writePipe == -1)
        throw std::runtime_error("Can't open named pipe for writing.");
      this->pipe_info.pipe1 = writePipe;
    }

    BUMP;

    if (rMode) {
      int flags = nonblocking ? O_RDONLY | O_NONBLOCK : O_RDONLY;
      int readPipe = ::open(path.c_str(), flags);
      BUMP;
      if (readPipe == -1)
        throw std::runtime_error("Can't open named pipe for reading.");
      this->pipe_info.pipe0 = readPipe;
    }

    this->pipeActive = true;

    BUMP;
  }

  ~NPipe() {
    this->cleanup();
    if (writeMode) {
      ::unlink(pipePath.c_str());
    }
  }

  void write(const T &msg) {
    if (!writeMode)
      throw std::runtime_error("This pipe isn't for writing.");

    Pipe<T, Length>::write(msg);
  }

  int read(T &msg) const {
    if (!readMode)
      throw std::runtime_error("This pipe isn't for reading.");
    return Pipe<T, Length>::read(msg);
  }
};

/**********************************************
  USINGS
**********************************************/
using SharedUPMessage = std::shared_ptr<UPipe<Message>>;
//=============================================

#endif /* UPipe_hpp */
