# Peer Link
Peer LinkはESP NowでのP2Pネットワークの構築を簡易化するライブラリです。

## 使用方法
### 初期化
`setup()`内で`peer_link_task_init(uint8_t channel, peer_id_t peer_id)`を呼び出してください
引数の`channel`はWiFiで使用するチャンネル1~14の値、`peer_id`はそのESP32のIDです。

`peer_id`は上位4bitが所属グループ、下位4bitがグループ内の識別IDとなっています。
`peer_id`が`0x0A`と`0x0B`のESP32は自動的にペアリングされますが、`0x0A`と`0x1B`だとグループが異なるためペアリングされません。

### 受信
メッセージを受信した場合`peer_link_recv_cb`という関数が呼ばれます。`main.cpp`内に定義してください

#### 例
```cpp
void peer_link_recv_cb(const peer_id_t peer_id, const std::vector<struct Message>& messages) {
    printf("id: 0x%02X\r\n", peer_id);
    for (struct Message message: messages) {
        printf("type = %d, data = ", peer_id, message.type);
        for (uint8_t byte : message.data) {
            printf("%02X, ", byte);
        }
        printf("\r\n");
    }
}
```

### 送信
`peer_link_send(const peer_id_t peer_id, const std::vector<struct Message>& messages)`を呼び出すと送信されます。
送信するデータはメッセージと呼ばれ、一回に複数メッセージを送信可能です。
`peer_id`は宛先のESP32のID、`messages`は送信したいデータを格納してださい
ブロードキャストしたい場合は`BROADCAST_ID`を`peer_id`にセットします

#### 例
```cpp
std::vector<struct Message> messages;
std::string str = "Hello World!";
struct Message message = {
    .type = 0xAB,
    .data = std::vector<uint8_t>(str.begin(), str.end())
};
messages.push_back(std::move(message));
peer_link_send(BROADCAST_ID, messages);
```

## メッセージについて
メッセージは送受信する基本単位です。
それぞれのメッセージは内容を識別するための値`type`を持っておりこの値を用いて`data`の中身を識別する形になります
`type`割り当てと`data`の構造は任意です

## ペアリングについて
同じドメインかつ、同じグループなら`peer_link_task_init`を呼んだあとは自動的にペアリングが行われます。
相手側が電源断や何かしらで通信ができない状況になったと判定するとペアリングは解除されます。
再度通信可能な状態に復帰すればまたペアリングされます。

同じドメインかはローカルMACアドレスの2~5バイト目を使用しており定義は`src/peer_link.h`にあるので適宜書き換えてください。
デフォルトでは`{ 'n',  'n',  'c',  't' }`となっています
グループの識別には`peer_id`の上位4bitを使用しています。
