#include "json_handler.h"

#include <ArduinoJson.h>
#include <cstring>
#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

#include "config.h"
#include "payload_codec.h"
#include "serial_emit.h"

static void sendOne(const Message &message) {
  if (!peer_link_is_peer_exist(TO_PEER_ID)) {
    emitLog("warn", "peer not found, drop message");
    return;
  }
  std::vector<Message> messages;
  messages.push_back(message);
  peer_link_send(TO_PEER_ID, messages);
}

static Message buildPosition(JsonObject payload) {
  TabletData_Pos pos = {static_cast<int16_t>(payload["x"].as<int>()),
                        static_cast<int16_t>(payload["y"].as<int>()),
                        static_cast<int16_t>(payload["direction"].as<int>())};
  return encodePayload(MessageType::Position, pos);
}

static Message buildGamepad(JsonObject payload) {
  GamepadData g;
  g.joystick_left.x = payload["joystick_left"]["x"].as<int>();
  g.joystick_left.y = payload["joystick_left"]["y"].as<int>();
  g.joystick_right.x = payload["joystick_right"]["x"].as<int>();
  g.joystick_right.y = payload["joystick_right"]["y"].as<int>();
  g.trigger_left = payload["trigger_left"].as<int>();
  g.trigger_right = payload["trigger_right"].as<int>();
  g.buttons.raw = payload["buttons"].as<unsigned int>();
  g.dpad = static_cast<Dpad>(payload["dpad"].as<int>());
  return encodePayload(MessageType::Gamepad, g);
}

void handleJsonLine(const String &line) {
  if (line.length() == 0) {
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    String msg = String("JSON parse failed: ") + error.c_str();
    emitLog("error", msg.c_str());
    emitResendRequest(error.c_str());
    return;
  }

  if (doc["type"].isNull()) {
    return;
  }
  const char *type = doc["type"];
  JsonObject payload = doc["payload"];

  if (strcmp(type, "position_update") == 0) {
    sendOne(buildPosition(payload));
  } else if (strcmp(type, "gamepad") == 0) {
    sendOne(buildGamepad(payload));
  } else if (strcmp(type, "gamepad_use") == 0) {
    sendOne(encodeEmpty(MessageType::GamePadUse));
  } else if (strcmp(type, "tablet_use") == 0) {
    sendOne(encodeEmpty(MessageType::TabletUse));
  } else if (strcmp(type, "load_belt") == 0) {
    sendOne(encodeEmpty(MessageType::LoadBelt));
  } else if (strcmp(type, "reload_belt") == 0) {
    sendOne(encodeEmpty(MessageType::ReloadBelt));
  } else if (strcmp(type, "reload_finish_belt") == 0) {
    sendOne(encodeEmpty(MessageType::ReloadFinishBelt));
  } else if (strcmp(type, "launch_belt") == 0) {
    sendOne(encodeEmpty(MessageType::LaunchBelt));
  } else {
    String msg = String("unknown type = ") + type;
    emitLog("warn", msg.c_str());
  }
}
