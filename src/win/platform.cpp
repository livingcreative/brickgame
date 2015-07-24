/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    https://github.com/livingcreative/brickgame

    feel free to use and modify
*/

// windows platform entry point and platform specific functions

#include <cstdio>
#include <cstdint>
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


// window class name
static const char *MAIN_WINDOW_CLASS = "TETRISFROMSCRATCH";


// Direct Input related stuff
static const size_t MAX_DEVICE_COUNT = 4;
static const size_t MAX_CONTROLLER_BUTTONS = 32;
static const size_t MAX_CONTROLLER_AXES = 8;
static const size_t MAX_CONTROLLER_POVS = 4;

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
struct ControllerState
{
    uint32_t buttons;
    int      axes[MAX_CONTROLLER_AXES];
    int      pov[MAX_CONTROLLER_POVS];
};


// do it in OOP style, but this is not necessary
class Win32Application
{
public:
    Win32Application(HINSTANCE hInstance)
    {
        // there's a chance that RegisterClassA() and/or CreateWindowExA()
        // call could fail, so their result should be checked
        // however for the 2nd step this is not needed

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
        mainwindow = CreateWindowExA(
            0, MAIN_WINDOW_CLASS, "Tetris from scratch", WS_OVERLAPPEDWINDOW,
            0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, nullptr
        );

        // set to window user data pointer to this Win32Application object
        SetWindowLongPtrA(mainwindow, GWLP_USERDATA, LONG_PTR(this));

        // initialize input (DirectX Input)
        input = nullptr;
        devlist.count = 0;
        memset(devstate, 0, sizeof(devstate));
        DirectInput8Create(
            hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8A,
            reinterpret_cast<LPVOID*>(&input), nullptr
        );
        // this is totally fine to continue without DirectInput, so
        // failure of DirectInput8Create is ignored
        if (input) {
            // enum joystick/gamepad devices
            input->EnumDevices(
                DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback,
                &devlist, DIEDFL_ALLDEVICES
            );

            // initialize all devices found
            for (size_t dev = 0; dev < devlist.count; ++dev) {
                IDirectInputDevice8A *device = nullptr;
                input->CreateDevice(devlist.devices[dev].giud, &device, nullptr);
                devlist.devices[dev].device = device;

                if (device) {
                    device->SetCooperativeLevel(
                        mainwindow,
                        DISCL_NONEXCLUSIVE | DISCL_FOREGROUND
                    );
                    device->SetDataFormat(&c_dfDIJoystick);
                }
            }
        }

        // actually here instead of SW_SHOWNORMAL parameter
        // nCmdShow should be used, but who cares?
        ShowWindow(mainwindow, SW_SHOWNORMAL);
    }

    ~Win32Application()
    {
        // clean up input
        if (input) {
            for (size_t dev = 0; dev < devlist.count; ++dev) {
                IDirectInputDevice8A *device = devlist.devices[dev].device;
                if (device) {
                    device->Release();
                }
            }

            // set devices count to 0 to prevent access to destroyed device objects
            // inside WndProc
            devlist.count = 0;

            input->Release();
        }

        // destroy main window
        // if "mainwindow" is 0 (there's some failure and window wasn't created)
        // it's ok to call DestroyWindow() with 0 handle value
        DestroyWindow(mainwindow);
    }

    void Run()
    {
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
                    if (device->GetDeviceState(sizeof(state), &state) == S_OK) {
                        for (size_t btn = 0; btn < MAX_CONTROLLER_BUTTONS; ++btn) {
                            uint32_t button_bit = 1 << btn;
                            bool newstate = state.rgbButtons[btn] >= 128;
                            bool oldstate = (devstate[dev].buttons & button_bit) != 0;

                            if (oldstate != newstate) {
                                DEBUGPrint("Controller #%i button #%i %s\n", dev, btn, newstate ? "down" : "up");
                            }

                            if (newstate) {
                                devstate[dev].buttons |= button_bit;
                            } else {
                                devstate[dev].buttons &= ~button_bit;
                            }
                        }

                        for (size_t pov = 0; pov < MAX_CONTROLLER_POVS; ++pov) {
                            int povvalue = state.rgdwPOV[pov];
                            if (devstate[dev].pov[pov] != povvalue) {
                                devstate[dev].pov[pov] = povvalue;
                                DEBUGPrint("Controller #%i pov #%i moved to %i\n", dev, pov, povvalue);
                            }
                        }

                        CheckControllerAxis(devstate[dev], dev, 0, state.lX);
                        CheckControllerAxis(devstate[dev], dev, 1, state.lY);
                        CheckControllerAxis(devstate[dev], dev, 2, state.lZ);
                        CheckControllerAxis(devstate[dev], dev, 3, state.lRx);
                        CheckControllerAxis(devstate[dev], dev, 4, state.lRy);
                        CheckControllerAxis(devstate[dev], dev, 5, state.lRz);
                        CheckControllerAxis(devstate[dev], dev, 6, state.rglSlider[0]);
                        CheckControllerAxis(devstate[dev], dev, 7, state.rglSlider[1]);
                    }
                }
            }

            // sleep 10ms
            Sleep(10);
        }
    }

private:
    // window callback function, used to react for system messages to window
    // it's static since Windows doesn't understand C++ methods
    // the advantage of making this functions class member - it has access to all
    // private members of the class
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        Win32Application *app = reinterpret_cast<Win32Application*>(
            GetWindowLongPtrA(hwnd, GWLP_USERDATA)
        );

        // process messages game needs
        switch (message) {
            case WM_CLOSE:
                // WM_CLOSE sent when user wants to close window
                // the PostQuitMessage() function posts WM_QUIT message in application
                // message queue
                PostQuitMessage(0);
                return 0;

            case WM_KILLFOCUS:
            case WM_SETFOCUS:
                // if window still has pointer to Win32Application instance - process
                // focus changes. DirectInput requires that devices should be "Acuired"
                // before getting their state
                if (app) {
                    for (size_t dev = 0; dev < app->devlist.count; ++dev) {
                        IDirectInputDevice8A *device = app->devlist.devices[dev].device;
                        if (device) {
                            message == WM_SETFOCUS ?
                                device->Acquire() : device->Unacquire();
                        }
                    }
                }
                return 0;
        }

        // process all other messages with system default handler
        return DefWindowProcA(hwnd, message, wparam, lparam);
    }

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

    // compare joystick/gamepad axis state and generate input event
    static void CheckControllerAxis(ControllerState &state, size_t joynum, size_t axisnumber, int axisvalue)
    {
        if (state.axes[axisnumber] != axisvalue) {
            state.axes[axisnumber] = axisvalue;
            DEBUGPrint("Controller #%i axis #%i moved to %i\n", joynum, axisnumber, axisvalue);
        }
    }

    // callback function for IDirectInput8A::EnumDevices, records devices to InputDeviceList
    // passed via pvRef
    static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
    {
        InputDeviceList *devlist = reinterpret_cast<InputDeviceList*>(pvRef);
        if (devlist->count < MAX_DEVICE_COUNT) {
            DEBUGPrint("Controller device #%i, \"%s\"\n", devlist->count, lpddi->tszProductName);

            devlist->devices[devlist->count].giud = lpddi->guidInstance;
            devlist->devices[devlist->count].device = nullptr;
            ++devlist->count;
        }

        return TRUE;
    }

private:
    HWND             mainwindow;

    IDirectInput8A  *input;
    InputDeviceList  devlist;
    ControllerState  devstate[MAX_DEVICE_COUNT];
};


// main entry point function, program execution starts here
int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine, int nCmdShow
)
{
    // Create application object instance
    Win32Application app(hInstance);
    // and run the main loop
    app.Run();

    return 0;
}
