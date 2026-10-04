#pragma once

#include <cstdint>
#include <cstring>
#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

template <typename T>
bool decodePayload(const std::vector<uint8_t> &data, T &out) {
  if (data.size() != sizeof(T)) {
    return false;
  }
  memcpy(&out, data.data(), sizeof(T));
  return true;
}

template <typename T>
Message encodePayload(MessageType type, const T &value) {
  const uint8_t *p = reinterpret_cast<const uint8_t *>(&value);
  return Message{static_cast<uint8_t>(type),
                 std::vector<uint8_t>(p, p + sizeof(T))};
}

inline Message encodeEmpty(MessageType type) {
  return Message{static_cast<uint8_t>(type), std::vector<uint8_t>()};
}
