#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <pthread.h>
#include "Datastructs.hpp"
#include "Pipes.hpp"

pthread_t startService(void* (*SERVICE)(void*), void* params);

void stopService(UPipe<Message> &pipe);

#endif // HELPERS_HPP
