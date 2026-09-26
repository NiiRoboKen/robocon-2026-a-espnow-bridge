#include <Arduino.h>
#include <peer_link.h>
#include <cstring>

const uint8_t WIFI_CHANNEL = 14;
const peer_id_t FROM_PEER_ID = 0x12;
const peer_id_t TO_PEER_ID = 0x11;

const char* TEXT = "Hello World!";

void peer_link_recv_cb(
  const peer_id_t peer_id,
  const std::vector<struct Message>& messages
) {
  printf("id: 0x%02X\r\n", peer_id);
  for (struct Message message : messages) {
    printf("type = %d, data = ", message.type);
    for (uint8_t byte : message.data) {
      printf("%02X, ", byte);
    }
    printf("\r\n");
  }
}

void setup() { peer_link_task_init(WIFI_CHANNEL, FROM_PEER_ID); }

void loop() {
  if (peer_link_is_peer_exist(TO_PEER_ID)) {
    std::vector<struct Message> messages;
    struct Message message = {
        .type = 0xAB, .data = std::vector<uint8_t>(TEXT, TEXT + std::strlen(TEXT))};
    messages.push_back(std::move(message));
    peer_link_send(TO_PEER_ID, messages);
  }
  delay(1000);
}
