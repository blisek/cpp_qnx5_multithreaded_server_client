#include "ServerFunctions.hpp"
#include "Structs.hpp"
#include "Datastructs.hpp"
#include "MessageType.hpp"
#include "Settings.hpp"
#include <iostream>
#include <string>
#include <cstring>
// #include <sstream>
#include <bitset>
#include <unistd.h>

void* service_nth_prime(void* params)
{
  if(!params) {
    std::cerr << "[S, Prime] Input & output pipe info null. Service will terminate.\n";
    return nullptr;
  }

  TwoWayPipeInfo *pipeInfo = static_cast<TwoWayPipeInfo*>(params);
  UPipe<Message>& inputPipe = pipeInfo->firstChannel();
  UPipe<Message>& outputPipe = pipeInfo->secondChannel();
  Message message;
  int number;

  while(true) {
    if(inputPipe->read(message) <= 0) {
      std::cerr << "[S, Prime] No bytes read/error occurred.\n";
      continue;
    }

    if(message.messageType == MessageType::STOP) {
      break;
    }

    number = message.load<int>(0u);
    std::cout << "[S, Prime] Prime received: " << number << '\n';
    message.messageType = MessageType::R_PRIME;
    message.store<bool>(number % 2 == 0, 0u);
    std::cout << "[S, Prime] Sending response for prime " << number << '\n';
    outputPipe.write(message);
    std::cout << "[S, Prime] Response for prime " << number << " sent.\n";
  }

  std::cout << std::endl;
  return nullptr;
}

void* service_string_xor(void* params)
{
  if(!params) {
    std::cerr << "[S, StringXor] Input & output pipe info null. Service will terminate.\n";
    return nullptr;
  }

  TwoWayPipeInfo* pipeInfo = static_cast<TwoWayPipeInfo*>(params);
  UPipe<Message>& inputPipe = pipeInfo->firstChannel();
  UPipe<Message>& outputPipe = pipeInfo->secondChannel();
  Message message;
  int messageId;
  std::string messageStr, messageStr2;
  const size_t messageMaxLength = message.length() - sizeof(int);
  char xoredString[messageMaxLength] = {};

  while(true) {
    // pierwszy lancuch
    inputPipe.read(message);
    messageId = message.load<int>(0u); // numer kontrolny
    messageStr = static_cast<char*>(message.load<int>(1u)); // lancuch tuz za liczba kontrolna

    // drugi lancuch
    inputPipe.read(message);
    messageStr2 = static_cast<char*>(message.load<int>(1u)); // drugi lancuch

    if(messageId != message.load<int>(0u)) {
      std::cerr << "[S, StringXor] Invalid message id.\n|\tFirst id: "
        << messageId << "\n|\tFirst message: " << messageStr
        << "\n|\tSecond id: " << message.load<int>(0u) << "\n|\tSecond message: "
        << messageStr2 << '\n';

      continue;
    }

    if(messageStr.length() >= messageMaxLength) {
      std::cerr << "[S, StringXor] Message 1 is too long.\n";
      continue;
    }

    if(messageStr2.length() >= messageMaxLength) {
      std::cerr << "[S, StringXor] Message 2 is too long.\n";
      continue;
    }

    if(messageStr.length() != messageStr2.length()) {
      std::cerr << "[S, StringXor] Two strings are unequal.\n|\tFirst message length: "
        << messageStr.length() << "\n|\tFirst message: " << messageStr
        << "\n|\tSecond message length: " << messageStr2.length()
        << "\n|\tSecond message: " << messageStr2 << '\n';

      continue;
    }

    ::memset(xoredString, 0, messageMaxLength);
    size_t lenMsg = messageStr.length() + 1 >= messageMaxLength-1 ?
      messageMaxLength-1 : messageStr.length() + 1;
    for(size_t i = 0; i < lenMsg; ++i) {
      xoredString[i] = messageStr[i] ^ messageStr2[i];
    }
    ::strncpy(message.ptr(0u), xoredString, lenMsg);
    message.messageType = MessageType::R_BXOR;
    outputPipe.write(message);
  }

  return nullptr;
}

void* service_dec2bin(void* params)
{
  if(!params) {
    std::cerr << "[S, StringXor] Input & output pipe info null. Service will terminate.\n";
    return nullptr;
  }

  TwoWayPipeInfo* pipeInfo = static_cast<TwoWayPipeInfo*>(params);
  UPipe<Message>& inputPipe = pipeInfo->firstChannel();
  UPipe<Message>& outputPipe = pipeInfo->secondChannel();
  Message message;
  int number;
  std::string binOutput;

  while(true) {
    inputPipe.read(message);
    number = message.load<int>(0u);

    binOutput = std::bitset<sizeof(int) * 8>(number).to_string();
    message.messageType = MessageType::R_DEC2BIN;
    size_t binOutputLength = binOutput.length();
    ::strncpy(message.ptr(0u), binOutput.c_str(),
      binOutputLength+1 < ::MESSAGE_LENGTH ? binOutputLength+1 : (::MESSAGE_LENGTH));
    outputPipe.write(message);
  }

  return nullptr;
}

void* service_responder(void* param)
{
  if(!param) {
    std::cerr << "[S, Responder] Argument param: expected ptr to _GlobalSettings, got nullptr.\n";
    return nullptr;
  }
  _GlobalSettings *pipes = static_cast<_GlobalSettings*>(param);
  Message message;
  int resp;

  while(true) {
    if(pipes->pipe_primary) {
      resp = pipes->pipe_primary->secondChannel().read(message);
      if(resp > 0) { // wpisano cos
        pipes->channel->respond(message);
        std::cout << "[S, Responder] Odpowiedziano wiadomoscia utworzona w pipe_primary o typie: " << message.messageType << '\n';
      }
    }

    if(message.messageType == MessageType::STOP)
      break;

    if(pipes->pipe_bxor) {
      resp = pipes->pipe_bxor->secondChannel().read(message);
      if(resp > 0) { // wpisano cos
        pipes->channel->respond(message);
        std::cout << "[S, Responder] Odpowiedziano wiadomoscia utworzona w pipe_bxor o typie: " << message.messageType << '\n';
      }
    }

    if(message.messageType == MessageType::STOP)
      break;

    if(pipes->pipe_dec2bin) {
      resp = pipes->pipe_dec2bin->secondChannel().read(message);
      if(resp > 0) { // wpisano cos
        pipes->channel->respond(message);
        std::cout << "[S, Responder] Odpowiedziano wiadomoscia utworzona w pipe_dec2bin o typie: " << message.messageType << '\n';
      }
    }

    if(message.messageType == MessageType::STOP)
      break;

    sleep(::RESPONDER_SLEEP_TIME_SECONDS);
  }

  std::cout << "[S, Responder] Stop working." << std::endl;
  return nullptr;
}
