#include <Arduino.h>
#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

#include "esp_now_io.h"
#include "message_codec.h"

static const unsigned long OUTPUT_INTERVAL_MS = 200;

static portMUX_TYPE storeMux = portMUX_INITIALIZER_UNLOCKED;

template <typename T> struct Latest {
  T value;
  bool dirty = false;
};

static Latest<StateData> latestState;
static Latest<TabletData_Pos> latestPosition;

template <typename T> static void store(Latest<T> &slot, const T &value) {
  portENTER_CRITICAL(&storeMux);
  slot.value = value;
  slot.dirty = true;
  portEXIT_CRITICAL(&storeMux);
}

template <typename T> static bool take(Latest<T> &slot, T &out) {
  bool had;
  portENTER_CRITICAL(&storeMux);
  had = slot.dirty;
  if (had) {
    out = slot.value;
    slot.dirty = false;
  }
  portEXIT_CRITICAL(&storeMux);
  return had;
}

void peer_link_recv_cb(const peer_id_t peer_id,
                       const std::vector<Message> &messages) {
  for (const Message &m : messages) {
    switch (static_cast<MessageType>(m.type)) {
    case MessageType::RobotState: {
      StateData s;
      if (decodePayload(m.data, s)) {
        store(latestState, s);
      } else {
        emitRaw(m.type, m.data);
      }
      break;
    }
    case MessageType::Position: {
      TabletData_Pos pos;
      if (decodePayload(m.data, pos)) {
        store(latestPosition, pos);
      } else {
        emitRaw(m.type, m.data);
      }
      break;
    }
    default:
      emitRaw(m.type, m.data);
      break;
    }
  }
}

void espNowOutputFlush() {
  static unsigned long lastOutput = 0;
  unsigned long now = millis();
  if (now - lastOutput < OUTPUT_INTERVAL_MS) {
    return;
  }
  lastOutput = now;

  StateData s;
  TabletData_Pos pos;
  if (take(latestState, s)) {
    emitStateData(s);
  }
  if (take(latestPosition, pos)) {
    emitPosition(pos);
  }
}
