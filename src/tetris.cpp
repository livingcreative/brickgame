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

        game runs in current platform environment
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
        p_box_y(10),
        p_box_speed_x(0),
        p_box_speed_y(0)
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

        float speed = 1000; // pixels per second

        // compute new speed vector
        p_box_speed_x = (float(input.joystick[0].axes[0]) / JOY_AXIS_MAX_VALUE * 2 - 1) * speed;
        p_box_speed_y = (float(input.joystick[0].axes[1]) / JOY_AXIS_MAX_VALUE * 2 - 1) * speed;

        if (input.keyboard.keys[KEY_LEFT] || input.joystick[0].povs[0] == JOY_DIRECTION_LEFT) {
            p_box_speed_x -= speed;
        }
        if (input.keyboard.keys[KEY_RIGHT] || input.joystick[0].povs[0] == JOY_DIRECTION_RIGHT) {
            p_box_speed_x += speed;
        }
        if (input.keyboard.keys[KEY_UP] || input.joystick[0].povs[0] == JOY_DIRECTION_UP) {
            p_box_speed_y -= speed;
        }
        if (input.keyboard.keys[KEY_DOWN] || input.joystick[0].povs[0] == JOY_DIRECTION_DOWN) {
            p_box_speed_y += speed;
        }
    }

    void Update(double interval)
    {
        // animate big rectangle position
        p_box_x += p_box_speed_x * float(interval);
        p_box_y += p_box_speed_y * float(interval);
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
    float p_box_speed_x;
    float p_box_speed_y;
};

// main platform source - contains platform entry point and platform specific
// functions
#include "platform.cpp"
