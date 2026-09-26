#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstdint>
#include <peer_link.h>

const uint8_t WIFI_CHANNEL = 14;
const peer_id_t FROM_PEER_ID = 0x11;
const peer_id_t TO_PEER_ID = 0x12;

const uint8_t MSG_TYPE_POSITION = 0x01;

struct PositionData {
  int16_t x = 0;
  int16_t y = 0;
  int16_t dir = 0;
};

// static PositionData targetPosition;

static String rxBuffer;

static const size_t JSON_CAPACITY = 512;

void handleJsonLine(const String &line) {
  if (line.length() == 0) {
    return;
  }

  Serial.print("[RAW] ");
  Serial.println(line);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, line);

  if (error) {
    Serial.print("[NG] JSON parse failed: ");
    Serial.println(error.c_str());
    Serial.print("     raw = ");
    Serial.println(line);
    return;
  }

  Serial.println("[OK] JSON received successfully");

  Serial.print("     data = ");
  serializeJson(doc, Serial);
  Serial.println();

  if (!doc["type"].isNull()) {
    const char *type = doc["type"];

    if (strcmp(type, "position_update") == 0 &&
        peer_link_is_peer_exist(TO_PEER_ID)) {
      JsonObject payload = doc["payload"];
      PositionData targetPosition = {payload["position"]["x"].as<int>(),
                                     payload["position"]["y"].as<int>(),
                                     payload["direction"].as<int>()};

      const uint8_t *p = reinterpret_cast<const uint8_t *>(&targetPosition);
      struct Message message = {
          .type = MSG_TYPE_POSITION,
          .data = std::vector<uint8_t>(p, p + sizeof(PositionData))};

      std::vector<struct Message> messages;
      messages.push_back(std::move(message));
      peer_link_send(TO_PEER_ID, messages);

    } else {
      Serial.print("     unknown type = ");
      Serial.println(type);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  rxBuffer.reserve(JSON_CAPACITY);

  peer_link_task_init(WIFI_CHANNEL, FROM_PEER_ID);

  Serial.println("ESP32 ready. Send JSON terminated by newline.");
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
        Serial.println("[NG] Input too long, buffer cleared.");
        rxBuffer = "";
      }
    }
  }
}
