#include <Arduino.h>
#include <ArduinoJson.h>

// 受信バッファ
static String rxBuffer;

// JSONドキュメントの容量
static const size_t JSON_CAPACITY = 512;

// 受信した1行分のJSONをパースして結果を出力
void handleJsonLine(const String &line) {
  // 空行は無視
  if (line.length() == 0) {
    return;
  }

  // パース前の生データ表示
  Serial.print("[RAW] ");
  Serial.println(line);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, line);

  if (error) {
    // パース失敗：エラー内容を出力
    Serial.print("[NG] JSON parse failed: ");
    Serial.println(error.c_str());
    Serial.print("     raw = ");
    Serial.println(line);
    return;
  }

  // パース成功：正常受信を通知
  Serial.println("[OK] JSON received successfully");

  // 受信内容を整形して出力
  Serial.print("     data = ");
  serializeJson(doc, Serial);
  Serial.println();

  // 例：特定のキーがあれば取り出して使う
  if (!doc["command"].isNull()) {
    const char *command = doc["command"];
    Serial.print("     command = ");
    Serial.println(command);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートの接続待ち（USB-CDC搭載ボード向け）
  }
  rxBuffer.reserve(JSON_CAPACITY);
  Serial.println("ESP32 ready. Send JSON terminated by newline.");
}

void loop() {
  // 受信データを1文字ずつ読み、改行までを1メッセージとして処理
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\n') {
      // 行末（\r\n対応のため末尾の\rを除去）
      rxBuffer.trim();
      handleJsonLine(rxBuffer);
      rxBuffer = "";
    } else {
      rxBuffer += c;

      // バッファあふれ防止
      if (rxBuffer.length() > JSON_CAPACITY) {
        Serial.println("[NG] Input too long, buffer cleared.");
        rxBuffer = "";
      }
    }
  }
}
