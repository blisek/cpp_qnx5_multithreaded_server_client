#include "Datastructs.hpp"
#include "Pipes.hpp"
#include "Settings.hpp"
#include "Channel.hpp"
#include "MessageType.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) try {
  Message outgoingMsg, incomingMsg;
  ChannelSettings channelSettings = loadStruct<ChannelSettings>(::CHANNEL_INFO_PATH);
  Channel<Message> channel(channelSettings.channel_id, channelSettings.parent_pid);
  int number;

  std::cout << "[C] Prosze podac liczba, ktora ma zostac zamieniona na postac binarna: ";
  std::cin >> number;

  outgoingMsg.messageType = MessageType::M_DEC2BIN;
  outgoingMsg.store<int>(number, 0u);
  std::cout << "[C] Czekanie na forme binarna liczby " << number << "...\n";
  channel.send(outgoingMsg, incomingMsg);

  if(incomingMsg.messageType == MessageType::ERROR) {
    std::cerr << "[C] Wystapil blad podczas wykonania polecenia: " << incomingMsg.message << '\n';
  } else if(incomingMsg.messageType == MessageType::R_DEC2BIN) {
    std::cout << "[C] Liczba " << number << " w postaci binarnej: " << incomingMsg.load<int>(0u) << '\n';
  } else {
    std::cerr << "[C] Otrzymano odpowiedź zwrotną o nieoczekiwanym typie: " << incomingMsg.messageType << '\n';
  }

  return EXIT_SUCCESS;
} catch (std::runtime_error err) {
  std::cerr << "[C] " << err.what() << std::endl;
}
