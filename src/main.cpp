#include <Arduino.h>
#include <ArduinoJson.h>

struct PositionData {
  int x = 0;
  int y = 0;
  int direction = 0;
};

static PositionData targetPosition;

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

    if (strcmp(type, "position_update") == 0) {
      JsonObject payload = doc["payload"];
      targetPosition.x = payload["position"]["x"].as<int>();
      targetPosition.y = payload["position"]["y"].as<int>();
      targetPosition.direction = payload["direction"].as<int>();

      Serial.print("     type = ");
      Serial.println(type);
      Serial.print("     position = (");
      Serial.print(targetPosition.x);
      Serial.print(", ");
      Serial.print(targetPosition.y);
      Serial.println(")");
      Serial.print("     direction = ");
      Serial.println(targetPosition.direction);
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
