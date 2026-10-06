#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

template <typename T> Message encodePayload(MessageType type, const T &value) {
  const uint8_t *p = reinterpret_cast<const uint8_t *>(&value);
  return Message{static_cast<uint8_t>(type),
                 std::vector<uint8_t>(p, p + sizeof(T))};
}

inline Message encodeEmpty(MessageType type) {
  return Message{static_cast<uint8_t>(type), std::vector<uint8_t>()};
}

template <typename T>
bool decodePayload(const std::vector<uint8_t> &data, T &out) {
  if (data.size() != sizeof(T)) {
    return false;
  }
  memcpy(&out, data.data(), sizeof(T));
  return true;
}

inline void emitLog(const char *level, const char *msg) {
  JsonDocument doc;
  doc["type"] = "log";
  doc["level"] = level;
  doc["msg"] = msg;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitResendRequest(const char *reason) {
  JsonDocument doc;
  doc["type"] = "resend_request";
  doc["reason"] = reason;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitStateData(const StateData &s) {
  JsonDocument doc;
  doc["type"] = "robot_state";
  JsonObject p = doc["payload"].to<JsonObject>();
  p["gamepad_used"] = s.gamepad_used;
  p["load_belt"] = s.load_belt;
  p["reload_belt"] = s.reload_belt;
  p["reload_finish_belt"] = s.reload_finish_belt;
  p["launch_belt"] = s.launch_belt;
  p["launch_pos_belt"] = s.launch_pos_belt;
  p["acc_pos_belt"] = s.acc_pos_belt;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitPosition(const TabletData_Pos &pos) {
  JsonDocument doc;
  doc["type"] = "position_update";
  JsonObject p = doc["payload"].to<JsonObject>();
  p["x"] = pos.x;
  p["y"] = pos.y;
  p["direction"] = pos.deg;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitRaw(uint8_t type, const std::vector<uint8_t> &data) {
  JsonDocument doc;
  doc["type"] = "raw";
  doc["msg_type"] = type;
  JsonArray arr = doc["data"].to<JsonArray>();
  for (uint8_t b : data) {
    arr.add(b);
  }
  serializeJson(doc, Serial);
  Serial.println();
}
