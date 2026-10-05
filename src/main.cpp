#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

#include "esp_now_io.h"
#include "message_codec.h"

static const size_t JSON_CAPACITY = 512;
static String rxBuffer;

static void sendOne(const Message &message) {
  if (!peer_link_is_peer_exist(SWERVE_S3_ID)) {
    emitLog("warn", "peer not found, drop message");
    return;
  }
  std::vector<Message> messages;
  messages.push_back(message);
  peer_link_send(SWERVE_S3_ID, messages);
}

static Message buildPosition(JsonObject payload) {
  TabletData_Pos pos = {static_cast<int16_t>(payload["x"].as<int>()),
                        static_cast<int16_t>(payload["y"].as<int>()),
                        static_cast<int16_t>(payload["direction"].as<int>())};
  return encodePayload(MessageType::Position, pos);
}

void handleJsonLine(const String &line) {
  if (line.length() == 0) {
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    emitLog("error", error.c_str());
    emitResendRequest(error.c_str());
    return;
  }
  if (doc["type"].isNull()) {
    return;
  }

  const char *type = doc["type"];
  JsonObject payload = doc["payload"];

  if (strcmp(type, "gamepad_use") == 0) {
    sendOne(encodeEmpty(MessageType::GamePadUse));
  } else if (strcmp(type, "tablet_use") == 0) {
    sendOne(encodeEmpty(MessageType::TabletUse));
  } else if (strcmp(type, "position_update") == 0) {
    sendOne(buildPosition(payload));
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

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  rxBuffer.reserve(JSON_CAPACITY);
  peer_link_task_init(WIFI_CHANNEL, TABLET_ESP_ID);
  emitLog("info", "ESP32 ready");
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      rxBuffer.trim();
      handleJsonLine(rxBuffer);
      rxBuffer = "";
    } else {
      rxBuffer += c;
      if (rxBuffer.length() > JSON_CAPACITY) {
        emitLog("error", "input too long, buffer cleared");
        rxBuffer = "";
      }
    }
  }
  espNowOutputFlush();
}
