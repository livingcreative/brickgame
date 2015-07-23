/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    https://github.com/livingcreative/brickgame

    feel free to use and modify
*/

// windows platform entry point source

#include <Windows.h>


// window class name
static const char *MAIN_WINDOW_CLASS = "TETRISFROMSCRATCH";


// do it in OOP style, but this is not necessary
class Win32Application
{
public:
    Win32Application(HINSTANCE hInstance)
    {
        // there's a chance that RegisterClassA() and/or CreateWindowExA()
        // call could fail, so their result should be checked
        // however for the 1st step this is not needed

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

        // actually here instead of SW_SHOWNORMAL parameter
        // nCmdShow should be used, but who cares?
        ShowWindow(mainwindow, SW_SHOWNORMAL);
    }

    ~Win32Application()
    {
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
        while (GetMessageA(&msg, 0, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

private:
    // window callback function, used to react for system messages to window
    // it's static since Windows doesn't understand C++ methods
    // the advantage of making this functions class member - it has access to all
    // private members of the class
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

private:
    HWND mainwindow;
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
