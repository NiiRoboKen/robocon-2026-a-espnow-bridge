#include <peer_link.h>
#include <robocon_2026_utility/include/message.h>
#include <vector>

#include "message_codec.h"

void peer_link_recv_cb(const peer_id_t peer_id,
                       const std::vector<Message> &messages) {
  for (const Message &m : messages) {
    switch (static_cast<MessageType>(m.type)) {
    case MessageType::RobotState: {
      StateData s;
      decodePayload(m.data, s) ? emitStateData(s) : emitRaw(m.type, m.data);
      break;
    }
    case MessageType::Position: {
      TabletData_Pos pos;
      decodePayload(m.data, pos) ? emitPosition(pos) : emitRaw(m.type, m.data);
      break;
    }
    case MessageType::LaunchStatus: {
      BeltElevationAccData d;
      decodePayload(m.data, d) ? emitLaunchStatus(d) : emitRaw(m.type, m.data);
      break;
    }
    default:
      emitRaw(m.type, m.data);
      break;
    }
  }
}
