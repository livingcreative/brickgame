/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// platform API definitions

#pragma once


#include <cstdint>


// input declarations

enum InputMouseButton
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_X1,
    MOUSE_BUTTON_X2,
    MOUSE_BUTTON_COUNT
};

enum InputKey
{
    KEY_BACK             = 0x08, // BACKSPACE key
    KEY_TAB              = 0x09, // TAB key
    KEY_RETURN           = 0x0D, // ENTER key
    KEY_PAUSE            = 0x13, // PAUSE key
    KEY_CAPITAL          = 0x14, // CAPS LOCK key
    KEY_ESCAPE           = 0x1B, // ESC key
    KEY_SPACE            = 0x20, // SPACEBAR
    KEY_PRIOR            = 0x21, // PAGE UP key
    KEY_NEXT             = 0x22, // PAGE DOWN key
    KEY_END              = 0x23, // END key
    KEY_HOME             = 0x24, // HOME key
    KEY_LEFT             = 0x25, // LEFT ARROW key
    KEY_UP               = 0x26, // UP ARROW key
    KEY_RIGHT            = 0x27, // RIGHT ARROW key
    KEY_DOWN             = 0x28, // DOWN ARROW key
    KEY_SNAPSHOT         = 0x2C, // PRINT SCREEN key
    KEY_INSERT           = 0x2D, // INS key
    KEY_DELETE           = 0x2E, // DEL key
    KEY_0                = 0x30,
    KEY_1                = 0x31,
    KEY_2                = 0x32,
    KEY_3                = 0x33,
    KEY_4                = 0x34,
    KEY_5                = 0x35,
    KEY_6                = 0x36,
    KEY_7                = 0x37,
    KEY_8                = 0x38,
    KEY_9                = 0x39,
    KEY_A                = 0x41,
    KEY_B                = 0x42,
    KEY_C                = 0x43,
    KEY_D                = 0x44,
    KEY_E                = 0x45,
    KEY_F                = 0x46,
    KEY_G                = 0x47,
    KEY_H                = 0x48,
    KEY_I                = 0x49,
    KEY_J                = 0x4A,
    KEY_K                = 0x4B,
    KEY_L                = 0x4C,
    KEY_M                = 0x4D,
    KEY_N                = 0x4E,
    KEY_O                = 0x4F,
    KEY_P                = 0x50,
    KEY_Q                = 0x51,
    KEY_R                = 0x52,
    KEY_S                = 0x53,
    KEY_T                = 0x54,
    KEY_U                = 0x55,
    KEY_V                = 0x56,
    KEY_W                = 0x57,
    KEY_X                = 0x58,
    KEY_Y                = 0x59,
    KEY_Z                = 0x5A,
    KEY_LWIN             = 0x5B, // Left Windows key (Natural keyboard)
    KEY_RWIN             = 0x5C, // Right Windows key (Natural keyboard)
    KEY_APPS             = 0x5D, // Applications key (Natural keyboard)
    KEY_NUMPAD0          = 0x60, // Numeric keypad 0 key
    KEY_NUMPAD1          = 0x61, // Numeric keypad 1 key
    KEY_NUMPAD2          = 0x62, // Numeric keypad 2 key
    KEY_NUMPAD3          = 0x63, // Numeric keypad 3 key
    KEY_NUMPAD4          = 0x64, // Numeric keypad 4 key
    KEY_NUMPAD5          = 0x65, // Numeric keypad 5 key
    KEY_NUMPAD6          = 0x66, // Numeric keypad 6 key
    KEY_NUMPAD7          = 0x67, // Numeric keypad 7 key
    KEY_NUMPAD8          = 0x68, // Numeric keypad 8 key
    KEY_NUMPAD9          = 0x69, // Numeric keypad 9 key
    KEY_MULTIPLY         = 0x6A, // Multiply key
    KEY_ADD              = 0x6B, // Add key
    KEY_SEPARATOR        = 0x6C, // Separator key
    KEY_SUBTRACT         = 0x6D, // Subtract key
    KEY_DECIMAL          = 0x6E, // Decimal key
    KEY_DIVIDE           = 0x6F, // Divide key
    KEY_F1               = 0x70, // F1 key
    KEY_F2               = 0x71, // F2 key
    KEY_F3               = 0x72, // F3 key
    KEY_F4               = 0x73, // F4 key
    KEY_F5               = 0x74, // F5 key
    KEY_F6               = 0x75, // F6 key
    KEY_F7               = 0x76, // F7 key
    KEY_F8               = 0x77, // F8 key
    KEY_F9               = 0x78, // F9 key
    KEY_F10              = 0x79, // F10 key
    KEY_F11              = 0x7A, // F11 key
    KEY_F12              = 0x7B, // F12 key
    KEY_NUMLOCK          = 0x90, // NUM LOCK key
    KEY_SCROLL           = 0x91, // SCROLL LOCK key
    KEY_LSHIFT           = 0xA0, // Left SHIFT key
    KEY_RSHIFT           = 0xA1, // Right SHIFT key
    KEY_LCONTROL         = 0xA2, // Left CONTROL key
    KEY_RCONTROL         = 0xA3, // Right CONTROL key
    KEY_LALT             = 0xA4, // Left ALT key
    KEY_RALT             = 0xA5, // Right ALT key
    KEY_VOLUME_MUTE      = 0xAD, // Volume Mute key
    KEY_VOLUME_DOWN      = 0xAE, // Volume Down key
    KEY_VOLUME_UP        = 0xAF, // Volume Up key
    KEY_MEDIA_NEXT_TRACK = 0xB0, // Next Track key
    KEY_MEDIA_PREV_TRACK = 0xB1, // Previous Track key
    KEY_MEDIA_STOP       = 0xB2, // Stop Media key
    KEY_MEDIA_PLAY_PAUSE = 0xB3, // Play/Pause Media key
    KEY_OEM_1            = 0xBA, // For the US standard keyboard, the ';:' key
    KEY_OEM_PLUS         = 0xBB, // For any country/region, the '+' key
    KEY_OEM_COMMA        = 0xBC, // For any country/region, the ',' key
    KEY_OEM_MINUS        = 0xBD, // For any country/region, the '-' key
    KEY_OEM_PERIOD       = 0xBE, // For any country/region, the '.' key
    KEY_OEM_2            = 0xBF, // For the US standard keyboard, the '/?' key
    KEY_OEM_3            = 0xC0, // For the US standard keyboard, the '`~' key
    KEY_OEM_4            = 0xDB, // For the US standard keyboard, the '[{' key
    KEY_OEM_5            = 0xDC, // For the US standard keyboard, the '\|' key
    KEY_OEM_6            = 0xDD, // For the US standard keyboard, the ']}' key
    KEY_OEM_7            = 0xDE, // For the US standard keyboard, the 'single-quote/double-quote' key
    KEY_OEM_8            = 0xDF, // Used for miscellaneous characters; it can vary by keyboard.
    KEY_QUIT             = 0xFF, // Special input command, platform quit request
    KEY_COUNT            = 256
};

enum InputKeyboardShift
{
    KEY_SHIFT   = 0x01,
    KEY_CONTROL = 0x02,
    KEY_ALT     = 0x04,
    KEY_NUM     = 0x08,
    KEY_CAPS    = 0x10
};

enum InputJoystickDeviceCount
{
    JOYSTICK_DEVICE_COUNT = 4
};

enum InputJoystickButton
{
    JOY_BUTTON_0,
    JOY_BUTTON_1,
    JOY_BUTTON_2,
    JOY_BUTTON_3,
    JOY_BUTTON_4,
    JOY_BUTTON_5,
    JOY_BUTTON_6,
    JOY_BUTTON_7,
    JOY_BUTTON_8,
    JOY_BUTTON_9,
    JOY_BUTTON_10,
    JOY_BUTTON_11,
    JOY_BUTTON_12,
    JOY_BUTTON_13,
    JOY_BUTTON_14,
    JOY_BUTTON_15,
    JOY_BUTTON_16,
    JOY_BUTTON_17,
    JOY_BUTTON_18,
    JOY_BUTTON_19,
    JOY_BUTTON_20,
    JOY_BUTTON_21,
    JOY_BUTTON_22,
    JOY_BUTTON_23,
    JOY_BUTTON_24,
    JOY_BUTTON_25,
    JOY_BUTTON_26,
    JOY_BUTTON_27,
    JOY_BUTTON_28,
    JOY_BUTTON_29,
    JOY_BUTTON_30,
    JOY_BUTTON_31,
    JOY_BUTTON_COUNT
};

enum InputJoystickAxis
{
    JOY_AXIS_0,
    JOY_AXIS_1,
    JOY_AXIS_2,
    JOY_AXIS_3,
    JOY_AXIS_4,
    JOY_AXIS_5,
    JOY_AXIS_6,
    JOY_AXIS_7,
    JOY_AXIS_COUNT
};

enum InputJoystickPOV
{
    JOY_POV_0,
    JOY_POV_1,
    JOY_POV_2,
    JOY_POV_3,
    JOY_POV_COUNT
};

enum InputJoystickPOVDirection
{
    JOY_DIRECTION_CENTER = -1,
    JOY_DIRECTION_UP = 0,
    JOY_DIRECTION_RIGHT = 9000,
    JOY_DIRECTION_DOWN = 18000,
    JOY_DIRECTION_LEFT = 27000
};

enum InputEventType
{
    INPUT_MOUSE_DOWN,  // mouse button down
    INPUT_MOUSE_UP,    // mouse button up
    INPUT_MOUSE_MOVE,  // mouse movement
    INPUT_MOUSE_WHEEL, // mouse wheel (scroll)
    INPUT_KEY_DOWN,    // keyboard key down
    INPUT_KEY_UP,      // keyboard key up
    INPUT_CHAR,        // keyboard typing character
    INPUT_BUTTON_DOWN, // joystick/gamepad button down
    INPUT_BUTTON_UP,   // joystick/gamepad button up
    INPUT_AXIS,        // joystick/gamepad axis change
    INPUT_POV          // joystick/gamepad POV change
};

enum InputEventCount
{
    INPUT_EVENT_COUNT = 64
};

struct InputMouseState
{
    int      x;
    int      y;
    uint32_t buttons;
};

struct InputKeyboardState
{
    uint8_t  keys[KEY_COUNT];
    uint32_t shifts;
};

struct InputJoystickState
{
    uint32_t buttons;
    int      axes[JOY_AXIS_COUNT];
    int      povs[JOY_POV_COUNT];
};

struct InputEventMouse
{
    int              x;
    int              y;
    InputMouseButton button;
    int              wheel;
};

struct InputEventKeyboard
{
    InputKey key;
};

struct InputEventJoystickAxis
{
    InputJoystickAxis axis;
    int               value;
};

struct InputEventJoystickPOV
{
    InputJoystickPOV pov;
    int              value;
};

struct InputEventJoystick
{
    uint32_t number;
    union {
        InputJoystickButton    button;
        InputEventJoystickAxis axis;
        InputEventJoystickPOV  pov;
    };
};

struct InputEvent
{
    InputEventType type;
    union {
        InputEventMouse    mouse;
        InputEventKeyboard keyboard;
        InputEventJoystick joystick;
    };
};

struct Input
{
    InputMouseState    mouse;
    InputKeyboardState keyboard;
    InputJoystickState joystick[JOYSTICK_DEVICE_COUNT];
    size_t             event_count;
    InputEvent         events[INPUT_EVENT_COUNT];
};


// graphics declarations

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color() :
        r(0), g(0), b(0), a(0)
    {}

    Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255) :
        r(_r), g(_g), b(_b), a(_a)
    {}
};


class PlatformAPI
{
public:
    virtual void Quit() = 0;

    virtual void DEBUGPrint(const char *format, ...) = 0;
};


class GraphicsAPI
{
public:
    virtual void Clear(const Color &color) = 0;
    virtual void Viewport(int left, int top, int width, int height) = 0;
    virtual void Rectangle(float left, float top, float width, float height, const Color &color) = 0;

protected:
    virtual void GetRenderTargetSize(int &width, int &height) = 0;
};
