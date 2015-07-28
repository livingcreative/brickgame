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
#include <gl/GL.h>


// window class name
static const char *MAIN_WINDOW_CLASS = "TETRISFROMSCRATCH";


// DirectInput related stuff

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
    InputDevice devices[CONTROLLER_DEVICE_COUNT];
};


// common platform functions which any platform might need
#include "platformcommon.cpp"


// do it in OOP style, but this is not necessary
class Win32Application : public PlatformAPI
{
public:
    Win32Application(HINSTANCE hInstance) :
        mainwindow(0),
        input(nullptr),
        gldc(0),
        glrc(0)
    {
        // initialization error flag
        bool initerror = false;

        devlist.count = 0;

        // this is "loop trick"
        // if some initialization step failed - just break to skip other parts
        // no exceptions, no crazy if-else checks
        for (;;) {
            // register main window class
            // window class describes window look and behaviour
            WNDCLASSA wcl = {};
            wcl.style         = CS_HREDRAW | CS_VREDRAW;
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

            // set to window user data pointer to this Win32Application object
            SetWindowLongPtrA(mainwindow, GWLP_USERDATA, LONG_PTR(this));

            // initialize input (DirectX Input)
            DirectInput8Create(
                hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8A,
                reinterpret_cast<LPVOID*>(&input), nullptr
            );
            // this is totally fine to continue without DirectInput, so
            // failure of DirectInput8Create is ignored
            if (input == nullptr) {
                // Direct input isn't critical part, game can be run without
                // any Direct Input devices, just log failure
                DEBUGPrint(
                    "Couldn't initialize DirectInput, "
                    "running game without game controller support!\n"
                );
            } else {
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

        running = !initerror;
        if (!initerror) {
            // actually here instead of SW_SHOWNORMAL parameter
            // nCmdShow should be used, but who cares?
            ShowWindow(mainwindow, SW_SHOWNORMAL);
        }
    }

    ~Win32Application()
    {
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
        if (!running) {
            return;
        }

        Input input = {};
        input.controller_count = devlist.count;

        TetrisGame game;

        // application main loop
        while (running) {
            // reset change flags in input structure
            input.mouse.xdelta = 0;
            input.mouse.ydelta = 0;
            for (size_t button = 0; button < MOUSE_BUTTON_COUNT; ++button) {
                input.mouse.buttons[button] &= ~BUTTON_CHANGED;
            }
            for (size_t key = 0; key < KEY_COUNT; ++key) {
                input.keyboard.keys[key] &= ~BUTTON_CHANGED;
            }
            for (size_t controller = 0; controller < input.controller_count; ++controller) {
                for (size_t button = 0; button < CONT_BUTTON_COUNT; ++button) {
                    input.controller[controller].buttons[button] &= ~BUTTON_CHANGED;
                }
            }

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
                        input.mouse.xdelta = pt->x - input.mouse.x;
                        input.mouse.ydelta = pt->y - input.mouse.y;
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
                    if (device->GetDeviceState(sizeof(state), &state) == S_OK) {
                        InputControllerState &cs = input.controller[dev];

                        for (size_t btn = 0; btn < CONT_BUTTON_COUNT; ++btn) {
                            SetKeyOrButtonState(state.rgbButtons[btn] >= 128, cs.buttons[btn]);
                        }

                        for (size_t pov = 0; pov < CONT_POV_COUNT; ++pov) {
                            int povvalue = state.rgdwPOV[pov];
                            cs.povsmoved[pov] = povvalue != cs.povs[pov];
                            cs.povs[pov] = povvalue;
                        }

                        CheckControllerAxis(input.controller[dev], CONT_AXIS_0, state.lX);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_1, state.lY);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_2, state.lZ);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_3, state.lRx);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_4, state.lRy);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_5, state.lRz);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_6, state.rglSlider[0]);
                        CheckControllerAxis(input.controller[dev], CONT_AXIS_7, state.rglSlider[1]);
                    }
                }
            }

            // pass input to game
            game.ProcessInput(*this, input);
            // render frame
            RenderGameFrame();

            // sleep 10ms
            Sleep(10);
        }
    }

    // platform API implementation
    void Quit() override
    {
        running = false;
    }

    void DEBUGPrint(const char *format, ...) override
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

            case WM_SYSKEYDOWN:
                // allow default processing for ALT+F4 combination
                if (wparam == VK_F4) {
                    break;
                }
                return 0;

            case WM_PAINT: {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);

                if (app) {
                    app->RenderGameFrame();
                }

                return 0;
            }
        }

        // process all other messages with system default handler
        return DefWindowProcA(hwnd, message, wparam, lparam);
    }

    // set mouse buttons state
    static void MouseButtonEvent(Input &input, InputMouseButton button, bool down)
    {
        SetKeyOrButtonState(down, input.mouse.buttons[button]);
    }

    // set keyboard state
    static void KeyboardEvent(Input &input, InputKey key, bool down)
    {
        SetKeyOrButtonState(down, input.keyboard.keys[key]);
    }

    // compare joystick/gamepad axis state and generate input event
    static void CheckControllerAxis(InputControllerState &state, InputControllerAxis axis, int axisvalue)
    {
        float newvalue = axisvalue <= 32767 ?
            float(axisvalue) / 32767 - 1 : float(axisvalue - 32768) / 32767;

        state.axesdelta[axis] = newvalue - state.axes[axis];
        state.axes[axis] = newvalue;
    }

    // callback function for IDirectInput8A::EnumDevices, records devices to InputDeviceList
    // passed via pvRef
    static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
    {
        InputDeviceList *devlist = reinterpret_cast<InputDeviceList*>(pvRef);
        if (devlist->count < CONTROLLER_DEVICE_COUNT) {
            devlist->devices[devlist->count].giud = lpddi->guidInstance;
            devlist->devices[devlist->count].device = nullptr;
            ++devlist->count;
        }
        return TRUE;
    }

    // render current game frame
    void RenderGameFrame()
    {
        // clear whole back buffer
        glClear(GL_COLOR_BUFFER_BIT);
        // display render result
        SwapBuffers(gldc);
    }

private:
    bool             running;
    HWND             mainwindow;

    IDirectInput8A  *input;
    InputDeviceList  devlist;
    HDC              gldc;
    HGLRC            glrc;
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
