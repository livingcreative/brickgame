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

// main platform source - contains platform entry point and platform specific
// functions
#include "platform.cpp"
