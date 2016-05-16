#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>

namespace {
const std::string INTERCHANGE_PIPE = "/tmp/cs_pipe";
const std::string CHANNEL_INFO_PATH = "/tmp/cs_channel";
const int MESSAGE_LENGTH = 80;
const unsigned RESPONDER_SLEEP_TIME_SECONDS = 1u;
}

#endif
