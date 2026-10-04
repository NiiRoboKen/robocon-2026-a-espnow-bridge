# シリアル ⇔ ESP-NOW ブリッジ

このファームウェアは、PC(ホスト) との **シリアル通信** と、他ESPとの **ESP-NOW通信**(peer-link ライブラリ経由) を相互に橋渡しする。

設定値は `include/config.h` にまとめている。

- このデバイスの peer_id: `FROM_PEER_ID = 0x12`
- 送信先の peer_id: `TO_PEER_ID = 0x11`
- Wi-Fiチャンネル: `WIFI_CHANNEL = 14`
- シリアルボーレート: `SERIAL_BAUD = 115200`

ペイロードの構造体定義は `include/robocon_2026_utility/include/message.h` を参照。
各メッセージは `MessageType`(1バイト) + 構造体を packed バイト列にしたものを peer-link の `Message` に載せて送受信する。

## ファイル構成

機能ごとにファイルを分割している。

| ファイル | 役割 |
| --- | --- |
| `src/main.cpp` | `setup` / `loop` のみ。各モジュールの初期化と駆動 |
| `include/config.h` | 設定定数(peer_id, channel, baud, バッファ長) |
| `include/payload_codec.h` | `encodePayload` / `decodePayload`(テンプレート) |
| `include/serial_emit.h` | 受信データをJSONにしてシリアル出力(`emit*`) |
| `include/json_handler.h` / `src/json_handler.cpp` | 方向1: シリアルJSON → ESP-NOW送信 |
| `src/peer_recv.cpp` | 方向2: ESP-NOW受信コールバックの実装 |
| `include/serial_reader.h` / `src/serial_reader.cpp` | シリアルの行バッファ読み取り |

## 方向1: シリアル受信 → ESP-NOW送信 (handleJsonLine)

ホスト(tablet)から **改行(`\n`)終端のJSON1行** を受け取り、解析してESP-NOWで送信先へ転送する。
各 `type` を `MessageType` に対応づけて送る。送信は送信先ペアが存在するとき
(`peer_link_is_peer_exist(TO_PEER_ID)`)のみ行い、不在時は `log`(warn)を返して破棄する。

| JSONの `type` | MessageType | ペイロード |
| --- | --- | --- |
| `position_update` | `Position` | `TabletData_Pos`(x, y, deg) |
| `gamepad` | `Gamepad` | `GamepadData` |
| `gamepad_use` | `GamePadUse` | なし |
| `tablet_use` | `TabletUse` | なし |
| `load_belt` | `LoadBelt` | なし |
| `reload_belt` | `ReloadBelt` | なし |
| `reload_finish_belt` | `ReloadFinishBelt` | なし |
| `launch_belt` | `LaunchBelt` | なし |

位置情報:

```json
{
  "type": "position_update",
  "payload": {
    "position": { "x": 100, "y": 200 },
    "direction": 90
  }
}
```

ゲームパッド:

```json
{
  "type": "gamepad",
  "payload": {
    "joystick_left": { "x": 0, "y": 0 },
    "joystick_right": { "x": 0, "y": 0 },
    "trigger_left": 0,
    "trigger_right": 0,
    "buttons": 0,
    "dpad": 8
  }
}
```

- `buttons` は `Buttons` の 16bit raw 値。`dpad` は `Dpad`(0..8)。
- ペイロードなしの型(`gamepad_use` 等)は `payload` を省略できる。
- JSONパース失敗は `log`(error) と `resend_request` を返す。未知の `type` は `log`(warn)。
- 1行が `JSON_CAPACITY`(512バイト) を超えた場合はバッファを破棄する。

## 方向2: ESP-NOW受信 → シリアル送信 (peer_link_recv_cb)

peer-link は受信フレームをデコードすると weak 関数 `peer_link_recv_cb(peer_id, messages)` を呼ぶ。
`src/peer_recv.cpp` でこれをオーバーライドし、受信した各 `Message` を **JSON1行** にしてシリアルへ出力する。

| MessageType | 出力 `type` | ペイロード構造体 |
| --- | --- | --- |
| `RobotState` | `robot_state` | `StateData` |
| `Position` | `position` | `TabletData_Pos` |
| `LaunchStatus` | `launch_status` | `BeltElevationAccData` |
| `Gamepad` | `gamepad` | `GamepadData` |

デコードできない型やサイズ不一致は生バイト列を `type: "raw"` で出力する。

```json
{
  "type": "robot_state",
  "payload": {
    "gamepad_used": false,
    "load_belt": false,
    "reload_belt": false,
    "reload_finish_belt": false,
    "launch_belt": false,
    "launch_pos_belt": 0,
    "acc_pos_belt": 0
  }
}
```

```json
{ "type": "position", "payload": { "x": 100, "y": 200, "deg": 90 } }
```

```json
{ "type": "launch_status", "payload": { "launch_pos_belt": 0, "acc_pos_belt": 0 } }
```

```json
{ "type": "raw", "msg_type": 33, "data": [1, 2, 3] }
```

## シリアル出力はすべてJSON1行

ログも含め、デバイスがシリアルへ出力する行は **すべて改行終端のJSON** で統一している。
受信側は「1行受信 → JSONパース → `type` で振り分け」の単純ループで処理でき、
ログとデータを確実に判別できる。

ログは `type: "log"` で出力する(`level` は `info` / `warn` / `error`)。

```json
{ "type": "log", "level": "error", "msg": "JSON parse failed: ..." }
```

出力される `type` の一覧:

- `log` : デバイスのログ(人間向けメッセージ)
- `robot_state` : `RobotState` の受信データ
- `position` : `Position` の受信データ
- `launch_status` : `LaunchStatus` の受信データ
- `gamepad` : `Gamepad` の受信データ
- `raw` : デコード未対応/サイズ不一致の受信データ
- `resend_request` : tablet → ESP のJSONパースに失敗したときの再送要求。tablet はこれを受けると直前の送信メッセージを再送する。

  ```json
  { "type": "resend_request", "reason": "InvalidInput" }
  ```

## 新しいメッセージ型の追加手順

1. `message.h` に `MessageType` の値と packed 構造体を追加する。
2. 送信方向: `handleJsonLine` に `type` 分岐を追加し、`encodePayload(MessageType::X, 構造体)` で `Message` を作って送る。
3. 受信方向: `peer_link_recv_cb` の `switch` に `case` を追加し、`decodePayload` でデコードして JSON を出力する。

## ヘルパ

- `encodePayload<T>(type, value)`(`payload_codec.h`) : packed 構造体を `Message` に変換する。
- `encodeEmpty(type)`(`payload_codec.h`) : ペイロードなしの `Message` を作る(制御メッセージ用)。
- `decodePayload<T>(data, out)`(`payload_codec.h`) : 受信バイト列をサイズ検証付きで構造体へ復元する(サイズ不一致なら false)。
- `emitLog(level, msg)`(`serial_emit.h`) : ログを `type: "log"` のJSONでシリアルへ出力する。
- `emitResendRequest(reason)`(`serial_emit.h`) : パース失敗時に `type: "resend_request"` を出力し tablet に再送を促す。
- `emitStateData` / `emitPosition` / `emitLaunchStatus` / `emitGamepad` / `emitRaw`(`serial_emit.h`) : 受信データをJSONにしてシリアルへ出力する。
