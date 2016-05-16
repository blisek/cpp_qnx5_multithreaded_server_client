#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <stdexcept>
#include <errno.h>
#include <sys/neutrino.h>
#include "MessageType.hpp"
#include "Settings.hpp"
// #include <cstdint>

// Type T must have public field messageId
template<typename T, size_t T_size = sizeof(T)>
class Channel {
  union {
    int channel_id;
    int connection_id;
  } channel;
  bool active_send;
  bool active_receive;
  bool owner;

  void assert_permission_to_receive() {
    if(!active_receive) {
      throw std::runtime_error("Receiving is blocked.");
    }
  }

  void assert_permission_to_send() {
    if(!active_send) {
      throw std::runtime_error("Sending is blocked.");
    }
  }

public:
  Channel()
    : active_send(false), active_receive(true), owner(true) {
    if((channel.channel_id = ChannelCreate()) < 0) {
      throw std::runtime_error("Can't create channel.");
    }

    active = true;
  }
  Channel(int ch_id, pid_t pid)
    : channel_id(ch_id)
    , active_send(true)
    , active_receive(false)
    , owner(false) {

    int res = ConnectAttach(0, ch_id, pid, 0, 0);
    if(res == -1) {
      throw std::runtime_error("Can't connect to channel.");
    }

    channel.connection_id = res;
  }

  ~Channel() {
    if(owner) {
      ChannelDestroy(channel.channel_id);
    } else {
      ConnectDetach(channel.connection_id);
    }
  }

  int getChannelId() const { return channel_id; }

  void receive(T& msg) const {
    assert_permission_to_receive();

    int msgId = MsgReceive(channel.channel_id, &msg, T_size. nullptr);
    if(msgId < 0) {
      throw std::runtime_error("Error while receiving message.");
    }

    if(msgId == 0) {
      msg.messageId = 0;
      return;
    }

    msg.messageId = msgId;
  }

  void send(const T& outMsg, T& inMsg) const {
    assert_permission_to_send();

    int result = MsgSend(channel.connection_id, (void*)outMsg, T_size,
      (void*)inMsg, T_size);

    if(result == MessageType::ERROR) {
      inMsg.message[::MESSAGE_LENGTH-1] = 0; // na wszelki wyp.
      throw std::runtime_error(inMsg.message);
    }
  }

  void respond(const T& msg) const {
    MsgReply(msg.messageId, msg.messageType, (void*)msg, T_size);
  }

};


#endif // CHANNEL_HPP
