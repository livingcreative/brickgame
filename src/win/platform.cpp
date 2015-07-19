/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// windows platform entry point and platform specific functions

#include <cstdio>
#include <cstdint>
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


// output platform debug information (only for testing)
static void DEBUGPrint(const char *format, ...)
{
#if defined(_DEBUG) || defined(DEBUG)
    va_list va;
    va_start(va, format);
    char buffer[1024];
    vsprintf_s(buffer, format, va);
    va_end(va);

    OutputDebugStringA(buffer);
#endif
}


// window class name
static const char *MAIN_WINDOW_CLASS = "TETRISFROMSCRATCH";

// window callback function, used to react for system messages to window
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    // process messages game needs
    switch (message) {
        case WM_CLOSE:
            // WM_CLOSE sent when user wants to close window
            // the PostQuitMessage() function posts WM_QUIT message in application
            // message queue
            PostQuitMessage(0);
            return 0;
    }

    // process all other messages with system default handler
    return DefWindowProcA(hwnd, message, wparam, lparam);
}


// Direct Input related stuff
static const size_t MAX_DEVICE_COUNT = 4;
static const size_t MAX_JOYSTICK_BUTTONS = 32;
static const size_t MAX_JOYSTICK_AXES = 8;
static const size_t MAX_JOYSTICK_POVS = 4;

// input device data
struct InputDevice
{
    GUID                  giud;
    IDirectInputDevice8A *device;
};

// list of input devices
struct InputDeviceList
{
    size_t      count;
    InputDevice devices[MAX_DEVICE_COUNT];
};

// joystick/gamepad state
struct JoystickState
{
    uint32_t buttons;
    int      axes[MAX_JOYSTICK_AXES];
    int      pov[MAX_JOYSTICK_POVS];
};

// compare joystick/gamepad axis state and generate input event
static void CheckJoyAxis(JoystickState &state, size_t joynum, size_t axisnumber, int axisvalue)
{
    if (state.axes[axisnumber] != axisvalue) {
        state.axes[axisnumber] = axisvalue;
        DEBUGPrint("Joystick #%i axis #%i moved to %i\n", joynum, axisnumber, axisvalue);
    }
}

// callback function for IDirectInput8A::EnumDevices, records devices to InputDeviceList
// passed via pvRef
static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
    InputDeviceList *devlist = reinterpret_cast<InputDeviceList*>(pvRef);
    if (devlist->count < MAX_DEVICE_COUNT) {
        DEBUGPrint("Joystick device #%i, \"%s\"\n", devlist->count, lpddi->tszProductName);

        devlist->devices[devlist->count].giud = lpddi->guidInstance;
        devlist->devices[devlist->count].device = nullptr;
        ++devlist->count;
    }

    return TRUE;
}


// main entry point function, program execution starts here
int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine, int nCmdShow
)
{
    // register main window class
    // window class describes window look and behaviour
    WNDCLASSA wcl = {};
    wcl.style         = CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc   = WndProc;
    wcl.hInstance     = hInstance;
    wcl.hIcon         = LoadIconA(0, IDI_APPLICATION);
    wcl.hCursor       = LoadCursorA(0, IDC_ARROW);
    wcl.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
    wcl.lpszClassName = MAIN_WINDOW_CLASS;
    RegisterClassA(&wcl);

    // create & show main window
    HWND mainwindow = CreateWindowExA(
        0, MAIN_WINDOW_CLASS, "Tetris from scratch", WS_OVERLAPPEDWINDOW,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, nullptr
    );

    // initialize input (DirectX Input)
    IDirectInput8A *input = nullptr;
    InputDeviceList devlist = {};
    JoystickState   joystate[MAX_DEVICE_COUNT] = {};
    DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<LPVOID*>(&input), nullptr);
    if (input) {
        // enum joystick/gamepad devices
        input->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, &devlist, DIEDFL_ALLDEVICES);

        // initialize all devices found
        for (size_t dev = 0; dev < devlist.count; ++dev) {
            IDirectInputDevice8A *device = nullptr;
            input->CreateDevice(devlist.devices[dev].giud, &device, nullptr);
            devlist.devices[dev].device = device;

            if (device) {
                device->SetCooperativeLevel(mainwindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
                device->SetDataFormat(&c_dfDIJoystick);
            }
        }
    }

    // actually here instead of SW_SHOWNORMAL parameter
    // nCmdShow should be used, but who cares?
    ShowWindow(mainwindow, SW_SHOWNORMAL);

    // application main loop
    // pulls out messages from application message queue and sends them to
    // WndProc
    // for now, GetMessage() function will return false only when it picks WM_QUIT message
    MSG msg;
    bool running = mainwindow != 0;
    while (running) {
        // pull out all system messages from queue
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);

            // process input from mouse/keyboard
            switch (msg.message) {
                case WM_MOUSEMOVE: {
                    POINTS *pt = reinterpret_cast<POINTS*>(&msg.lParam);
                    DEBUGPrint("Mouse moved: %i %i\n", pt->x, pt->y);
                    break;
                }

                case WM_LBUTTONDOWN:
                    DEBUGPrint("Mouse left button down\n");
                    break;

                case WM_LBUTTONUP:
                    DEBUGPrint("Mouse left button up\n");
                    break;

                case WM_RBUTTONDOWN:
                    DEBUGPrint("Mouse right button down\n");
                    break;

                case WM_RBUTTONUP:
                    DEBUGPrint("Mouse right button up\n");
                    break;

                case WM_KEYDOWN:
                    DEBUGPrint("Key #%i down\n", msg.wParam);
                    break;

                case WM_KEYUP:
                    DEBUGPrint("Key #%i up\n", msg.wParam);
                    break;
            }
        }

        // process input from joystick/gamepad
        for (size_t dev = 0; dev < devlist.count; ++dev) {
            IDirectInputDevice8A *device = devlist.devices[dev].device;
            if (device) {
                // get current device state
                DIJOYSTATE state = {};
                if (device->GetDeviceState(sizeof(state), &state) != S_OK) {
                    device->Acquire();
                    device->GetDeviceState(sizeof(state), &state);
                }

                for (size_t btn = 0; btn < MAX_JOYSTICK_BUTTONS; ++btn) {
                    uint32_t button_bit = 1 << btn;
                    bool newstate = state.rgbButtons[btn] >= 128;
                    bool oldstate = (joystate[dev].buttons & button_bit) != 0;

                    if (oldstate != newstate) {
                        DEBUGPrint("Joystick #%i button #%i %s\n", dev, btn, newstate ? "down" : "up");
                    }

                    if (newstate) {
                        joystate[dev].buttons |= button_bit;
                    } else {
                        joystate[dev].buttons &= ~button_bit;
                    }
                }

                for (size_t pov = 0; pov < MAX_JOYSTICK_POVS; ++pov) {
                    int povvalue = state.rgdwPOV[pov];
                    if (joystate[dev].pov[pov] != povvalue) {
                        joystate[dev].pov[pov] = povvalue;
                        DEBUGPrint("Joystick #%i pov #%i moved to %i\n", dev, pov, povvalue);
                    }
                }

                CheckJoyAxis(joystate[dev], dev, 0, state.lX);
                CheckJoyAxis(joystate[dev], dev, 1, state.lY);
                CheckJoyAxis(joystate[dev], dev, 2, state.lZ);
                CheckJoyAxis(joystate[dev], dev, 3, state.lRx);
                CheckJoyAxis(joystate[dev], dev, 4, state.lRy);
                CheckJoyAxis(joystate[dev], dev, 5, state.lRz);
                CheckJoyAxis(joystate[dev], dev, 6, state.rglSlider[0]);
                CheckJoyAxis(joystate[dev], dev, 7, state.rglSlider[1]);
            }
        }

        // sleep 10ms
        Sleep(10);
    }

    // clean up input
    if (input) {
        for (size_t dev = 0; dev < devlist.count; ++dev) {
            IDirectInputDevice8A *device = devlist.devices[dev].device;
            if (device) {
                device->Unacquire();
                device->Release();
            }
        }

        input->Release();
    }

    // destroy main window
    DestroyWindow(mainwindow);

    return 0;
}
