#include "serial_reader.h"

#include <Arduino.h>

#include "config.h"
#include "json_handler.h"
#include "serial_emit.h"

static String rxBuffer;

void serialReaderInit() { rxBuffer.reserve(JSON_CAPACITY); }

void serialReaderPoll() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      rxBuffer.trim();
      handleJsonLine(rxBuffer);
      rxBuffer = "";
    } else {
      rxBuffer += c;
      if (rxBuffer.length() > JSON_CAPACITY) {
        emitLog("error", "Input too long, buffer cleared.");
        rxBuffer = "";
      }
    }
  }
}
