#pragma once
#include <cstdint>

enum class MessageType {
    RobotState  = 0x01,
    TabletPos   = 0x02,
    Gamepad     = 0x03,
    TabletOrder = 0x04,
};

struct JoyStick {
        int8_t x;
        int8_t y;
};

union Buttons {
        uint16_t raw;
        struct {
                uint16_t north : 1;
                uint16_t east : 1;
                uint16_t south : 1;
                uint16_t west : 1;
                uint16_t joystick_left : 1;
                uint16_t joystick_right : 1;
                uint16_t shoulder_left : 1;
                uint16_t shoulder_right : 1;
                uint16_t trigger_left : 1;
                uint16_t trigger_right : 1;
                uint16_t start : 1;
                uint16_t select : 1;
                uint16_t __reserved : 4;
        } bits;
};

enum class Dpad : uint8_t {
    Up        = 0,
    RightUp   = 1,
    Right     = 2,
    RightDown = 3,
    Down      = 4,
    LeftDown  = 5,
    Left      = 6,
    LeftUp    = 7,
    Neutral   = 8
};

struct __attribute__((packed)) GamepadData {
        struct JoyStick joystick_left;
        struct JoyStick joystick_right;
        uint8_t         trigger_left;
        uint8_t         trigger_right;
        union Buttons   buttons;
        enum Dpad       dpad;
};

struct __attribute__((packed)) TabletData_Pos {
        int16_t x;
        int16_t y;
        int16_t deg;
};
struct __attribute__((packed)) TabletOrder {
        bool     gamepad_use;
        bool     belt_launch;
        bool     roller_launch;
        bool     floor_collection_open;
        bool     bucket_collection_open;
        int16_t  belt_launch_deg;
        uint16_t bucket_collection_height;
        uint16_t bucket_collection_front_back;
};
struct __attribute__((packed)) StateData {
        bool     gamepad_used;
        bool     is_launch_belt;
        bool     is_launch_roller;
        bool     floor_rag_collection;
        int16_t  belt_launch_deg;
        uint16_t Bucket_collection_height;
        uint16_t Bucket_collection_front_back;
};