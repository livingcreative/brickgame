/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// windows platform entry point source

#include <Windows.h>


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
    // actually here instead of SW_SHOWNORMAL parameter
    // nCmdShow should be used, but who cares?
    ShowWindow(mainwindow, SW_SHOWNORMAL);

    // application main loop
    // pulls out messages from application message queue and sends them to
    // WndProc
    // for now, GetMessage() function will return false only when it picks WM_QUIT message
    MSG msg;
    while (GetMessageA(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    // destroy main window
    DestroyWindow(mainwindow);

    return 0;
}
