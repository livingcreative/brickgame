/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// this is only file which is passed for compiler
// it will include all other sources

/*
    general structure
        game does 3 things:
            1. gather user input
            2. do some processing of current game state
            3. output audio and video

        item #2 is actually game code
        items #1, #3 are common for any game in general

        game runs in current platform invioronment
        so, platform specific code "wraps" game code
        following diagram shows interaction between game code and platform code

        +-------------------------------------+
        |    platform specific entry point    |
        +-------------------------------------+    +-------------------------------------+
        | sets up all neccessary stuff to run |    |            Platform API             |
        | game, creates main window and       |    +-------------------------------------+
        | initializes input/output APIs       |    | some platform functions exposed to  |
        | runs main loop                      |    | game code (I/O, threads)            |
        +-------------------------------------+    +-------------------------------------+
                           |                                          ^
                           V                                          |
        +-------------------------------------+    +-------------------------------------+
        |   main loop (platform specific)     |    |   game code (platform independed)   |
        +-------------------------------------+    +-------------------------------------+
        | collects system messages and input  |--->| does all game processing            |
        | passes processed input data to game |    | calls back some platform API code   |
        +-------------------------------------+    | to create output, read files and    |
                                                   | do ther platform related stuff      |
                                                   +-------------------------------------+
*/

#include "engine/engine.h"
#include "platform/platform.h"


// game class
class Game
{
public:
    Game() :
        p_mouse_x(0),
        p_mouse_y(0),
        p_box_x(10),
        p_box_y(10)
    {}

    void ProcessInput(PlatformAPI &api, const Input &input)
    {
        // check for ESC key for quit
        if (input.keyboard.keys[KEY_ESCAPE]) {
            api.Quit();
        }

        // copy mouse coords to internal fields so
        // rectangle could follow mouse
        p_mouse_x = float(input.mouse.x);
        p_mouse_y = float(input.mouse.y);

        // where to move big rectangle?
        bool left = false;
        bool up = false;
        bool right = false;
        bool down = false;

        // debug output all events
        for (size_t e = 0; e < input.event_count; ++e) {
            const InputEvent &event = input.events[e];
            switch (event.type) {
                case INPUT_MOUSE_DOWN:
                    api.DEBUGPrint("Mouse %i button down\n", event.mouse.button);
                    break;

                case INPUT_MOUSE_UP:
                    api.DEBUGPrint("Mouse %i button up\n", event.mouse.button);
                    break;

                case INPUT_MOUSE_MOVE:
                    api.DEBUGPrint("Mouse moved: %i %i\n", event.mouse.x, event.mouse.y);
                    break;

                case INPUT_MOUSE_WHEEL:
                    break;

                case INPUT_KEY_DOWN:
                    api.DEBUGPrint("Key %i down\n", event.keyboard.key);

                    // move rectangle by keyboard arrow keys
                    switch (event.keyboard.key) {
                        case KEY_UP: up = true; break;
                        case KEY_DOWN: down = true; break;
                        case KEY_LEFT: left = true; break;
                        case KEY_RIGHT: right = true; break;
                    }

                    break;

                case INPUT_KEY_UP:
                    api.DEBUGPrint("Key %i up\n", event.keyboard.key);
                    break;

                case INPUT_CHAR:
                    break;

                case INPUT_BUTTON_DOWN:
                    api.DEBUGPrint(
                        "Joystick %i button %i down\n",
                        event.joystick.number, event.joystick.button
                    );
                    break;

                case INPUT_BUTTON_UP:
                    api.DEBUGPrint(
                        "Joystick %i button %i up\n",
                        event.joystick.number, event.joystick.button
                    );
                    break;

                case INPUT_AXIS:
                    api.DEBUGPrint(
                        "Joystick %i axis %i moved to %i\n",
                        event.joystick.number, event.joystick.axis.axis,
                        event.joystick.axis.value
                    );
                    break;

                case INPUT_POV:
                    api.DEBUGPrint(
                        "Joystick %i pov %i moved to %i\n",
                        event.joystick.number, event.joystick.pov.pov,
                        event.joystick.pov.value
                    );

                    // move rectangle by joystick POV
                    switch (event.joystick.pov.value) {
                        case JOY_DIRECTION_CENTER: up = down = left = right = false; break;
                        case JOY_DIRECTION_UP: up = true; break;
                        case JOY_DIRECTION_RIGHT: right = true; break;
                        case JOY_DIRECTION_DOWN: down = true; break;
                        case JOY_DIRECTION_LEFT: left = true; break;
                    }

                    break;
            }
        }

        // move big rectangle
        if (left) {
            p_box_x -= 10;
        }
        if (right) {
            p_box_x += 10;
        }
        if (up) {
            p_box_y -= 10;
        }
        if (down) {
            p_box_y += 10;
        }
    }

    void RenderGraphics(GraphicsAPI &api, int width, int height)
    {
        api.Clear(Color(20, 40, 205));

        // big rectangle, reacts to some user input
        api.Rectangle(p_box_x, p_box_y, 100, 100, Color(255, 50, 20));

        // tiny mouse rectangle, just to show mouse following
        api.Rectangle(p_mouse_x - 5, p_mouse_y - 5, 10, 10, Color(255, 255, 255));
    }

private:
    float p_mouse_x;
    float p_mouse_y;

    float p_box_x;
    float p_box_y;
};

// main platform source - contains platform entry point and platform specific
// functions
#include "platform.cpp"
