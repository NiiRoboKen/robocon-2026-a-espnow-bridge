#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstdint>
#include <robocon_2026_utility/include/message.h>
#include <vector>

inline void emitLog(const char *level, const char *msg) {
  JsonDocument doc;
  doc["type"] = "log";
  doc["level"] = level;
  doc["msg"] = msg;
  serializeJson(doc, Serial);
  Serial.println();
}

// tablet → ESP のJSON受信に失敗したことを tablet へ伝える。
// tablet はこれを受け取ると直前に送ったメッセージを再送する。
// reason には失敗理由(人間向け)を入れる。
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
  doc["type"] = "position";
  JsonObject p = doc["payload"].to<JsonObject>();
  p["x"] = pos.x;
  p["y"] = pos.y;
  p["deg"] = pos.deg;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitLaunchStatus(const BeltElevationAccData &d) {
  JsonDocument doc;
  doc["type"] = "launch_status";
  JsonObject p = doc["payload"].to<JsonObject>();
  p["launch_pos_belt"] = d.launch_pos_belt;
  p["acc_pos_belt"] = d.acc_pos_belt;
  serializeJson(doc, Serial);
  Serial.println();
}

inline void emitGamepad(const GamepadData &g) {
  JsonDocument doc;
  doc["type"] = "gamepad";
  JsonObject p = doc["payload"].to<JsonObject>();
  p["joystick_left"]["x"] = g.joystick_left.x;
  p["joystick_left"]["y"] = g.joystick_left.y;
  p["joystick_right"]["x"] = g.joystick_right.x;
  p["joystick_right"]["y"] = g.joystick_right.y;
  p["trigger_left"] = g.trigger_left;
  p["trigger_right"] = g.trigger_right;
  p["buttons"] = g.buttons.raw;
  p["dpad"] = static_cast<uint8_t>(g.dpad);
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
