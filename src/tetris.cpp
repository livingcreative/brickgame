/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    https://github.com/livingcreative/brickgame

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
        |   main loop (platform specific)     |    |   game code (platform independent)  |
        +-------------------------------------+    +-------------------------------------+
        | collects system messages and input  |--->| does all game processing            |
        | passes processed input data to game |    | calls back some platform API code   |
        +-------------------------------------+    | to create output, read files and    |
                                                   | do other platform related stuff     |
                                                   +-------------------------------------+
*/

#include "platform/platform.h"


// game class
class Game : public GameInterface
{
public:
    void ProcessInput(PlatformAPI &api, const Input &input) override
    {
        // check for ESC key for quit
        if (input.keyboard.keys[KEY_ESCAPE] & BUTTON_DOWN) {
            api.Quit();
            return;
        }

        // debug output all events
        if (input.mouse.wasmoved) {
            api.DEBUGPrint(
                "Mouse moved to: %i %i (deltas: %i %i)\n",
                input.mouse.x, input.mouse.y, input.mouse.xdelta, input.mouse.ydelta
            );
        }

        for (size_t mousebtn = 0; mousebtn < MOUSE_BUTTON_COUNT; ++mousebtn) {
            if (input.mouse.buttons[mousebtn] & BUTTON_CHANGED) {
                api.DEBUGPrint(
                    "Mouse %i button %s\n",
                    mousebtn,
                    input.mouse.buttons[mousebtn] & BUTTON_DOWN ? "down" : "up"
                );
            }
        }

        for (size_t key = 0; key < KEY_COUNT; ++key) {
            if (input.keyboard.keys[key] & BUTTON_CHANGED) {
                api.DEBUGPrint(
                    "Key %i %s\n",
                    key,
                    input.keyboard.keys[key] & BUTTON_DOWN ? "down" : "up"
                );
            }
        }

        for (size_t controller = 0; controller < CONTROLLER_DEVICE_COUNT; ++controller) {
            const InputControllerState &cs = input.controller[controller];

            for (size_t btn = 0; btn < CONT_BUTTON_COUNT; ++btn) {
                if (cs.buttons[btn] & BUTTON_CHANGED) {
                    api.DEBUGPrint(
                        "Controller %i button %i %s\n",
                        controller, btn,
                        cs.buttons[btn] & BUTTON_DOWN ? "down" : "up"
                    );
                }
            }

            for (size_t axis = 0; axis < CONT_AXIS_COUNT; ++axis) {
                if (cs.axesdelta[axis] != 0) {
                    api.DEBUGPrint(
                        "Controller %i axis %i moved to %.2f (delta: %.2f)\n",
                        controller, axis,
                        cs.axes[axis], cs.axesdelta[axis]
                    );
                }
            }

            for (size_t pov = 0; pov < CONT_POV_COUNT; ++pov) {
                if (cs.povsmoved[pov]) {
                    api.DEBUGPrint(
                        "Controller %i POV %i moved to %i\n",
                        controller, pov, cs.povs[pov]
                    );
                }
            }
        }
    }
};

// main platform source - contains platform entry point and platform specific
// functions
#include "platform.cpp"
