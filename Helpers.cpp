#include "Helpers.hpp"
#include "MessageType.hpp"
#include <pthread.h>
#include <errno.h>
#include <stdexcept>

pthread_t startService(void* (*SERVICE)(void*), void* params)
{
  if(!SERVICE) {
    std::cerr << "[S, startService] Pointer to service is null!\n";
    return;
  }

  pthread_t threadInfo = {};
  if(!pthread_create(&threadInfo, NULL, SERVICE, params)) {
    throw std::runtime_error("Can't create thread");
  }

  return threadInfo;
}

void stopService(UPipe<Message> &pipe)
{
  Message msg;
  msg.messageType = MessageType::STOP;
  pipe.write(msg);
}
