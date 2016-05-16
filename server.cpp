#include "Datastructs.hpp"
#include "MessageType.hpp"
#include "Pipes.hpp"
#include "Settings.hpp"
#include "Structs.hpp"
#include "Helpers.hpp"
#include "ServerFunctions.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <memory>

// ustawienia i dane o potokach
// Structs.hpp
_GlobalSettings globalSettings;

// pilnuje by usługi zostały odpowiednio
// zainicjowane i pozniej zakonczone
struct ServiceGuard {
  ServiceGuard() {
    startService(service_nth_prime, globalSettings.pipe_primary.get());
    startService(service_string_xor, globalSettings.pipe_bxor.get());
    startService(service_dec2bin, globalSettings.pipe_dec2bin.get());
  }
  ~ServiceGuard() {
    stopService(globalSettings.pipe_primary->firstChannel());
    stopService(globalSettings.pipe_bxor->firstChannel());
    stopService(globalSettings.pipe_dec2bin->firstChannel());
  }
};

void dispatcher();

int main(int argc, char **argv) try {
  ServiceGuard serviceGuard;
  writeStruct(globalSettings.channelSettings, CHANNEL_INFO_PATH);
  startService(service_responder, &globalSettings);

  dispatcher();

  std::cout << "[S] Server stopping." << std::endl;
  return EXIT_SUCCESS;
} catch (std::runtime_error err) {
  std::cerr << "[S] " << err.what() << std::endl;
}

void dispatcher() {
  Message message = { MessageType::OK };
  while(message.messageType != MessageType::STOP) {
    try {
      channel->receive(message);

      switch(message.messageType) {
      case MessageType::M_PRIME:
        globalSettings.pipe_primary.firstChannel().write(message);
        break;
      case MessageType::M_BXOR:
        globalSettings.pipe_bxor.firstChannel().write(message);
        break;
      case MessageType::M_DEC2BIN:
        globalSettings.pipe_dec2bin.firstChannel().write(message);
        break;
      case MessageType::ERROR:
        std::cerr << "[S, Dispatcher] Received error message from client: "
          << message.message << '\n';
        break;
      case MessageType::STOP:
        std::cout << "[S, Dispatcher] Stop message received.\n";
        globalSettings.pipe_primary.secondChannel().write(message);
        break;
      default:
        std::cerr << "[S, Dispatcher] Unknown type of message received: " << message.messageType
          << ". Dispatcher will continue its work.\n";
        break;
      }

    } catch(std::runtime_error err) {
      std::cerr << "[S, Dispatcher] Error occurred: " << err.what() << '\n';
    }
  }
}
