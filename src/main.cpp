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

static Message buildBeltLaunch(JsonObject payload) {
  int16_t acceleration = {
      static_cast<int16_t>(payload["acceleration"].as<int>()),
  };
  return encodePayload(MessageType::BeltLaunch, acceleration);
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

  if (strcmp(type, "target_position") == 0) {
    sendOne(buildPosition(payload));
    return;
  }
  if (strcmp(type, "belt_launch") == 0) {
    sendOne(buildBeltLaunch(payload));
    return;
  }

  static const struct {
    const char *name;
    MessageType type;
  } commands[] = {
      {"gamepad_use", MessageType::GamePadUse},
      {"tablet_use", MessageType::TabletUse},
      {"reboot", MessageType::Reboot},
      {"belt_load", MessageType::BeltLoad},
      {"belt_reload", MessageType::BeltReload},
      {"belt_reload_finish", MessageType::BeltReloadFinish},
      {"belt_desk", MessageType::BeltDesk},
      {"belt_bucket_low", MessageType::BeltBucket_Low},
      {"belt_bucket_middle", MessageType::BeltBucket_Middle},
      {"belt_bucket_high", MessageType::BeltBucket_High},
      {"belt_frag", MessageType::BeltFrag},
      {"belt_elevation", MessageType::BeltElevation},
      {"roller_start", MessageType::RollerStart},
      {"roller_launch", MessageType::RollerLaunch},
      {"bucket_low", MessageType::BucketLow},
      {"bucket_middle", MessageType::BucketMiddle},
      {"bucket_high", MessageType::BucketHigh},
      {"bucket_release", MessageType::BucketRelease},
      {"floor_on", MessageType::FloorOn},
      {"floor_off", MessageType::FloorOff},
  };

  for (const auto &cmd : commands) {
    if (strcmp(type, cmd.name) == 0) {
      sendOne(encodeEmpty(cmd.type));
      return;
    }
  }

  String msg = String("unknown type = ") + type;
  emitLog("warn", msg.c_str());
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
