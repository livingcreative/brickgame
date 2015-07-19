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
#include <gl/GL.h>
#include "platform/platform.h"


// output platform debug information (only for testing)
static void DEBUGPrintVA(const char *format, va_list va)
{
#if defined(_DEBUG) || defined(DEBUG)
    char buffer[1024];
    vsprintf_s(buffer, format, va);
    OutputDebugStringA(buffer);
#endif
}

static void DEBUGPrint(const char *format, ...)
{
#if defined(_DEBUG) || defined(DEBUG)
    va_list va;
    va_start(va, format);
    DEBUGPrintVA(format, va);
    va_end(va);
#endif
}


// common engine functions and implementation
#include "engine.cpp"


// platform API implementation
class WindowsPlatform : public PlatformAPI, public OpenGLAPI
{
public:
    void Quit() override
    {
        PostQuitMessage(0);
    }

    void DEBUGPrint(const char *format, ...) override
    {
#if defined(_DEBUG) || defined(DEBUG)
        va_list va;
        va_start(va, format);
        DEBUGPrintVA(format, va);
        va_end(va);
#endif
    }

    void UpdateRenderTargetSize(int width, int height)
    {
        p_rt_width = width;
        p_rt_height = height;
    }

protected:
    void GetRenderTargetSize(int &width, int &height) override
    {
        width = p_rt_width;
        height = p_rt_height;
    }

private:
    int p_rt_width;
    int p_rt_height;
};


// function for complete game frame render
// used in main loop and WndProc WM_PAINT message to update window contents
// while doing system ops such as moving or resizing which block main loop
static void RenderGameFrame(WindowsPlatform &api, HWND mainwindow, HDC gldc, Game &game)
{
    // set full window viewport for testing
    RECT rc;
    GetClientRect(mainwindow, &rc);
    api.UpdateRenderTargetSize(rc.right, rc.bottom);
    glViewport(0, 0, rc.right, rc.bottom);

    if (rc.right && rc.bottom) {
        // set default ortho projection matrix for 2D rendering
        GLfloat projection[16] = {
            2 / float(rc.right), 0, 0, 0,
            0, -2 / float(rc.bottom), 0, 0,
            0, 0, 1, 0,
            -1, 1, 0, 1
        };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);

        // ask game to render
        game.RenderGraphics(api, rc.right, rc.bottom);

        // display render result
        SwapBuffers(gldc);
    }
}


// window class name
static const char *MAIN_WINDOW_CLASS = "TETRISFROMSCRATCH";

// data passed to window, used to access some objects inside WndProc
struct WindowData
{
    HDC             *gldc;
    WindowsPlatform *api;
    Game            *game;
};

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

        case WM_PAINT: {
            WindowData *data = reinterpret_cast<WindowData*>(
                GetWindowLongPtrA(hwnd, GWLP_USERDATA)
            );

            if (data == nullptr || data->api == nullptr || data->game == nullptr) {
                // break to default processing
                break;
            } else {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);

                RenderGameFrame(*data->api, hwnd, *data->gldc, *data->game);

                return 0;
            }
        }

        case WM_SYSKEYDOWN:
            if (wparam == VK_F4) {
                break;
            }
            return 0;
    }

    // process all other messages with system default handler
    return DefWindowProcA(hwnd, message, wparam, lparam);
}


// Direct Input related stuff

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
    InputDevice devices[JOYSTICK_DEVICE_COUNT];
};

// generate mouse button events and set corresponding state
static void MouseButtonEvent(Input &input, InputMouseButton button, bool down)
{
    if (InputEvent *event = new_event(input)) {
        event->type = down ? INPUT_MOUSE_DOWN : INPUT_MOUSE_UP;
        event->mouse.button = button;
        event->mouse.wheel = 0;
        event->mouse.x = input.mouse.x;
        event->mouse.y = input.mouse.y;
    }

    if (down) {
        input.mouse.buttons |= (1 << button);
    } else {
        input.mouse.buttons &= ~(1 << button);
    }
}

// generate keyboard event and set state
static void KeyboardEvent(Input &input, InputKey key, bool down)
{
    if (InputEvent *event = new_event(input)) {
        event->type = down ? INPUT_KEY_DOWN : INPUT_KEY_UP;
        event->keyboard.key = key;
    }

    if (down) {
        input.keyboard.keys[key] = 1;
    } else {
        input.keyboard.keys[key] = 0;
    }

    switch (key) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            input.keyboard.keys[KEY_LSHIFT] || input.keyboard.keys[KEY_RSHIFT] ?
                input.keyboard.shifts |= KEY_SHIFT : input.keyboard.shifts &= ~KEY_SHIFT;
            break;

        case KEY_LCONTROL:
        case KEY_RCONTROL:
            input.keyboard.keys[KEY_LCONTROL] || input.keyboard.keys[KEY_RCONTROL] ?
                input.keyboard.shifts |= KEY_CONTROL : input.keyboard.shifts &= ~KEY_CONTROL;
            break;

        case KEY_LALT:
        case KEY_RALT:
            input.keyboard.keys[KEY_LALT] || input.keyboard.keys[KEY_RALT] ?
                input.keyboard.shifts |= KEY_ALT : input.keyboard.shifts &= ~KEY_ALT;
            break;

        case KEY_NUMLOCK:
            GetKeyState(VK_NUMLOCK) & 1 ?
                input.keyboard.shifts |= KEY_NUM : input.keyboard.shifts &= ~KEY_NUM;
            break;

        case KEY_CAPITAL:
            GetKeyState(VK_CAPITAL) & 1 ?
                input.keyboard.shifts |= KEY_CAPS : input.keyboard.shifts &= ~KEY_CAPS;
            break;
    }
}

// compare joystick/gamepad axis state and generate input event
static void CheckJoyAxis(Input &input, size_t joynum, size_t axisnumber, int axisvalue)
{
    if (input.joystick[joynum].axes[axisnumber] != axisvalue) {
        input.joystick[joynum].axes[axisnumber] = axisvalue;
        if (InputEvent *event = new_event(input)) {
            event->type = INPUT_AXIS;
            event->joystick.number = joynum;
            event->joystick.axis.axis = InputJoystickAxis(axisnumber);
            event->joystick.axis.value = axisvalue;
        }
    }
}

// callback function for IDirectInput8A::EnumDevices, records devices to InputDeviceList
// passed via pvRef
static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
    InputDeviceList *devlist = reinterpret_cast<InputDeviceList*>(pvRef);
    if (devlist->count < JOYSTICK_DEVICE_COUNT) {
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
    // initialization error flag
    bool initerror = false;

    HWND mainwindow = 0;
    IDirectInput8A *input = nullptr;
    InputDeviceList devlist = {};
    HDC gldc = 0;
    HGLRC glrc = 0;

    LARGE_INTEGER frequency;

    WindowData data = { &gldc };

    // this is "loop trick"
    // if some initialization step failed - just break to skip other parts
    // no exceptions, no crazy if-else checks
    for (;;) {

        // register main window class
        // window class describes window look and behaviour
        WNDCLASSA wcl = {};
        wcl.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcl.lpfnWndProc   = WndProc;
        wcl.hInstance     = hInstance;
        wcl.hIcon         = LoadIconA(0, IDI_APPLICATION);
        wcl.hCursor       = LoadCursorA(0, IDC_ARROW);
        wcl.hbrBackground = 0;
        wcl.lpszClassName = MAIN_WINDOW_CLASS;
        RegisterClassA(&wcl);

        // create & show main window
        mainwindow = CreateWindowExA(
            0, MAIN_WINDOW_CLASS, "Tetris from scratch", WS_OVERLAPPEDWINDOW,
            0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, nullptr
        );

        if (mainwindow == 0) {
            DEBUGPrint("Couldn't create main window!\n");
            initerror = true;
            break;
        }

        // set window data
        SetWindowLongPtrA(mainwindow, GWLP_USERDATA, LONG(&data));

        // initialize timer frequency
        QueryPerformanceFrequency(&frequency);

        // initialize input (DirectX Input)
        DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<LPVOID*>(&input), nullptr);
        if (input == nullptr) {
            // Direct input isn't critical part, game can be run without
            // any Direct Input devices, just log failure
            DEBUGPrint(
                "Couldn't initialize DirectInput, "
                "running game without game controller support!\n"
            );
        } else {
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

        // initialize OpenGL
        // OpenGL initialization is platform specific, rest of the OpenGL is not
        // so context initialization done in platform layer
        gldc = GetDC(mainwindow);
        if (gldc == 0) {
            DEBUGPrint("Couldn't get device context of main window!\n");
            initerror = true;
            break;
        }

        // use default "old" pixel format initialization for now
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 0; // unless game is 3D don't use Z-buffer
        int pfn = ChoosePixelFormat(gldc, &pfd);
        if (pfn == 0) {
            DEBUGPrint("Couldn't obtain OpenGL pixel format for main window!\n");
            initerror = true;
            break;
        }

        if (!SetPixelFormat(gldc, pfn, nullptr)) {
            DEBUGPrint("Couldn't set OpenGL pixel format for main window!\n");
            initerror = true;
            break;
        }

        // create and set context
        glrc = wglCreateContext(gldc);
        if (glrc == 0) {
            DEBUGPrint("Couldn't create OpenGL context!\n");
            initerror = true;
            break;
        }
        wglMakeCurrent(gldc, glrc);

        // just for testing, set nice background color
        glClearColor(0.2f, 0.4f, 1.0f, 1.0f);

        // not to repeat forever loop
        break;
    }

    if (!initerror) {
        // actually here instead of SW_SHOWNORMAL parameter
        // nCmdShow should be used, but who cares?
        ShowWindow(mainwindow, SW_SHOWNORMAL);

        // application main loop
        // pulls out messages from application message queue and sends them to
        // WndProc
        // for now, GetMessage() function will return false only when it picks WM_QUIT message
        Input input = {};
        input.keyboard.shifts =
            (GetKeyState(VK_SHIFT) < 0 ? KEY_SHIFT : 0) |
            (GetKeyState(VK_CONTROL) < 0 ? KEY_CONTROL : 0) |
            (GetKeyState(VK_MENU) < 0 ? KEY_ALT : 0) |
            (GetKeyState(VK_CAPITAL) & 1 ? KEY_CAPS : 0) |
            (GetKeyState(VK_NUMLOCK) & 1 ? KEY_NUM : 0);

        WindowsPlatform api;
        Game game;

        // update window data structure
        data.api = &api;
        data.game = &game;

        LARGE_INTEGER lasttime;
        QueryPerformanceCounter(&lasttime);

        bool running = mainwindow != 0;
        while (running) {
            // query current time to get interval last frame took
            LARGE_INTEGER currenttime;
            QueryPerformanceCounter(&currenttime);

            // reset event count, events passed by frame basis
            input.event_count = 0;

            // pull out all system messages from queue
            MSG msg;
            while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                }

                TranslateMessage(&msg);
                DispatchMessageA(&msg);

                // process input from mouse/keyboard and some other messages
                switch (msg.message) {
                    case WM_MOUSEMOVE: {
                        POINTS *pt = reinterpret_cast<POINTS*>(&msg.lParam);

                        if (InputEvent *event = new_event(input)) {
                            event->type = INPUT_MOUSE_MOVE;
                            event->mouse.button = MOUSE_BUTTON_COUNT;
                            event->mouse.wheel = 0;
                            event->mouse.x = pt->x;
                            event->mouse.y = pt->y;
                        }

                        input.mouse.x = pt->x;
                        input.mouse.y = pt->y;

                        break;
                    }

                    case WM_LBUTTONDOWN:
                        MouseButtonEvent(input, MOUSE_BUTTON_LEFT, true);
                        break;

                    case WM_LBUTTONUP:
                        MouseButtonEvent(input, MOUSE_BUTTON_LEFT, false);
                        break;

                    case WM_RBUTTONDOWN:
                        MouseButtonEvent(input, MOUSE_BUTTON_RIGHT, true);
                        break;

                    case WM_RBUTTONUP:
                        MouseButtonEvent(input, MOUSE_BUTTON_RIGHT, false);
                        break;

                    case WM_SYSKEYDOWN:
                    case WM_KEYDOWN:
                        KeyboardEvent(input, InputKey(msg.wParam), true);
                        break;

                    case WM_SYSKEYUP:
                    case WM_KEYUP:
                        KeyboardEvent(input, InputKey(msg.wParam), false);
                        break;
                }
            }

            // process input from joystick/gamepad
            for (size_t dev = 0; dev < devlist.count; ++dev) {
                IDirectInputDevice8A *device = devlist.devices[dev].device;
                if (device) {
                    // get current device state
                    DIJOYSTATE state = {};
                    bool statereceived = true;
                    if (device->GetDeviceState(sizeof(state), &state) != S_OK) {
                        device->Acquire();
                        statereceived = device->GetDeviceState(sizeof(state), &state) == S_OK;;
                    }

                    if (statereceived) {
                        for (size_t btn = 0; btn < JOY_BUTTON_COUNT; ++btn) {
                            uint32_t button_bit = 1 << btn;
                            bool newstate = state.rgbButtons[btn] >= 128;
                            bool oldstate = (input.joystick[dev].buttons & button_bit) != 0;

                            if (oldstate != newstate) {
                                if (InputEvent *event = new_event(input)) {
                                    event->type = newstate ? INPUT_BUTTON_DOWN : INPUT_BUTTON_UP;
                                    event->joystick.number = dev;
                                    event->joystick.button = InputJoystickButton(btn);
                                }
                            }

                            if (newstate) {
                                input.joystick[dev].buttons |= button_bit;
                            } else {
                                input.joystick[dev].buttons &= ~button_bit;
                            }
                        }

                        for (size_t pov = 0; pov < JOY_POV_COUNT; ++pov) {
                            int povvalue = state.rgdwPOV[pov];
                            if (input.joystick[dev].povs[pov] != povvalue) {
                                input.joystick[dev].povs[pov] = povvalue;
                                if (InputEvent *event = new_event(input)) {
                                    event->type = INPUT_POV;
                                    event->joystick.number = dev;
                                    event->joystick.pov.pov = InputJoystickPOV(pov);
                                    event->joystick.pov.value = povvalue;
                                }
                            }
                        }

                        CheckJoyAxis(input, dev, JOY_AXIS_0, state.lX);
                        CheckJoyAxis(input, dev, JOY_AXIS_1, state.lY);
                        CheckJoyAxis(input, dev, JOY_AXIS_2, state.lZ);
                        CheckJoyAxis(input, dev, JOY_AXIS_3, state.lRx);
                        CheckJoyAxis(input, dev, JOY_AXIS_4, state.lRy);
                        CheckJoyAxis(input, dev, JOY_AXIS_5, state.lRz);
                        CheckJoyAxis(input, dev, JOY_AXIS_6, state.rglSlider[0]);
                        CheckJoyAxis(input, dev, JOY_AXIS_7, state.rglSlider[1]);
                    }
                }
            }

            // pass input to game
            game.ProcessInput(api, input);

            // update game state (and animations)
            game.Update(
                double(currenttime.QuadPart - lasttime.QuadPart) /
                double(frequency.QuadPart)
            );
            lasttime = currenttime;

            // render game graphics
            RenderGameFrame(api, mainwindow, gldc, game);

            // sleep 10ms
            Sleep(10);
        }
    }

    // clean up OpenGL
    if (glrc) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(glrc);
    }

    if (gldc) {
        ReleaseDC(mainwindow, gldc);
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
