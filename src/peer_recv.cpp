#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

#include "payload_codec.h"
#include "serial_emit.h"

void peer_link_recv_cb(const peer_id_t peer_id,
                       const std::vector<Message> &messages) {
  for (const Message &m : messages) {
    switch (static_cast<MessageType>(m.type)) {
    case MessageType::RobotState: {
      StateData s;
      if (decodePayload(m.data, s)) {
        emitStateData(s);
      } else {
        emitRaw(m.type, m.data);
      }
      break;
    }
    case MessageType::Position: {
      TabletData_Pos pos;
      if (decodePayload(m.data, pos)) {
        emitPosition(pos);
      } else {
        emitRaw(m.type, m.data);
      }
      break;
    }
    case MessageType::LaunchStatus: {
      BeltElevationAccData d;
      if (decodePayload(m.data, d)) {
        emitLaunchStatus(d);
      } else {
        emitRaw(m.type, m.data);
      }
      break;
    }
    case MessageType::Gamepad: {
      GamepadData g;
      if (decodePayload(m.data, g)) {
        emitGamepad(g);
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
