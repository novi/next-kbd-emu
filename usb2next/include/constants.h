#ifndef CONSTANTS_h
#define CONSTANTS_h

#define DEBUG 1

#define KEY_BREAK 0x80
#define VALID_KEYCODE 0x80

#define NEXT_KB_CONTROL 0x1
#define NEXT_KB_ALT_LEFT 0x20
#define NEXT_KB_ALT_RIGHT 0x40
#define NEXT_KB_COMMAND_LEFT 0x8
#define NEXT_KB_COMMAND_RIGHT 0x10
#define NEXT_KB_SHIFT_LEFT 0x2
#define NEXT_KB_SHIFT_RIGHT 0x4

const uint8_t usbToNextKeycodes[] = {
    0, // 0
    0, // 1
    0, // 2
    0, // 3

    0x39, // KEY_A 0x04
    0x35,
    0x33,
    0x3b,
    0x44,
    0x3c,
    0x3d,
    0x40,
    0x06,
    0x3f,
    0x3e,
    0x2d,
    0x36,
    0x37,
    0x07,
    0x08, // KEY_P
    0x42,
    0x45,
    0x3a,
    0x48,
    0x46,
    0x34,
    0x43,
    0x32,
    0x47,
    0x31, // KEY_Z

    0x4a, // KEY_1,
    0x4b,
    0x4c,
    0x4d,
    0x50,
    0x4f,
    0x4e,
    0x1e,
    0x1f,
    0x20, // KEY_0

    0x2a, // KEY_ENTER
    0x49, // KEY_ESC
    0x1b, // KEY_BACKSPACE
    0x41, // KEY_TAB
    0x38, // KEY_SPACE
    0x1d, // KEY_MINUS
    0x1c, // KEY_EQUAL
    0x05, // KEY_LEFTBRACE
    0x04, // KEY_RIGHTBRACE
    0x03, // KEY_BACKSLASH
    0,// KEY_HASHTILDE // Keyboard Non-US # and ~
    0x2c, // KEY_SEMICOLON
    0x2b, // KEY_APOSTROPHE
    0x26, // KEY_GRAVE
    0x2e, // KEY_COMMA
    0x2f, // KEY_DOT
    0x30, // KEY_SLASH
    0, // KEY_CAPSLOCK, // TODO: map to control

    0, // KEY_F1
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // KEY_F12

    0, // KEY_SYSRQ
    0, // KEY_SCROLLLOCK
    0, // KEY_PAUSE
    0, // KEY_INSERT
    0, // KEY_HOME
    0, // KEY_PAGEUP
    0, // KEY_DELETE
    0, // KEY_END
    0, // KEY_PAGEDOWN

    0x10, // KEY_RIGHT
    0x09, // KEY_LEFT
    0x0f, // KEY_DOWN
    0x16, // KEY_UP 

    0, // KEY_NUMLOCK
    0x28, // KEY_KPSLASH
    0x25, // KEY_KPASTERISK
    0x24, // KEY_KPMINUS
    0x15, // KEY_KPPLUS
    0x0d, // KEY_KPENTER
    0x11, // KEY_KP1
    0x17, // KEY_KP2
    0x14, // KEY_KP3
    0x12, // KEY_KP4
    0x18, // KEY_KP5
    0x13, // KEY_KP6
    0x21, // KEY_KP7
    0x22, // KEY_KP8
    0x23, // KEY_KP9
    0x0b, // KEY_KP0
    0x0c, // KEY_KPDOT
    
};

#endif
