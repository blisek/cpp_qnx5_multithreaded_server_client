#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include "Datastructs.hpp"
#include "Pipes.hpp"
#include "Settings.hpp"
#include "Channel.hpp"
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <process.h>

struct TwoWayPipeInfo {
  UPipe<Message> firstChannel_;
  UPipe<Message> secondChannel_;

public:
  TwoWayPipeInfo(bool firstNonBlocking = false, bool secondNonBlocking = false)
    : firstChannel_(firstNonBlocking), secondChannel_(secondNonBlocking)
  {}

  UPipe<Message>& firstChannel() { return firstChannel_; }
  UPipe<Message>& secondChannel() { return secondChannel_; }
};

struct _GlobalSettings {
  ChannelSettings channelSettings;
  std::unique_ptr<TwoWayPipeInfo> pipe_primary;
  std::unique_ptr<TwoWayPipeInfo> pipe_bxor;
  std::unique_ptr<TwoWayPipeInfo> pipe_dec2bin;
  std::unique_ptr<Channel<Message>> channel;

  _GlobalSettings()
    : channelSettings()
    , pipe_primary(new TwoWayPipeInfo(false, true))
    , pipe_bxor(new TwoWayPipeInfo(false, true))
    , pipe_dec2bin(new TwoWayPipeInfo(false, true))
    , channel(new Channel<Message>()) {

      channelSettings.parent_pid = getpid();
      channelSettings.channel_id = channel.getChannelId();
    }
};

#endif // STRUCTS_HPP
