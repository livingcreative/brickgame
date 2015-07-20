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

#include <cstdlib>
#include "engine/engine.h"
#include "platform/platform.h"


// Figure class
enum FigureType
{
    None,
    Stick,
    Box,
    LeftL,
    RightL,
    LeftZ,
    RightZ,
    T
};

class Figure
{
public:
    Figure() :
        p_type(None),
        p_width(0),
        p_height(0)
    {}

    void Make(FigureType type)
    {
        p_type = type;
        switch (type) {
            case Stick:
                p_width = 1;
                p_height = 4;
                for (int n = 0; n < 4; ++n) {
                    p_data[n] = Color(220, 100, 50);
                }
                break;

            case Box:
                p_width = 2;
                p_height = 2;
                for (int n = 0; n < 4; ++n) {
                    p_data[n] = Color(100, 220, 50);
                }
                break;

            case LeftL:
                p_width = 2;
                p_height = 3;
                for (int n = 0; n < 6; ++n) {
                    p_data[n] = (n & 1) && n < 4 ?
                        Color(0, 0, 0, 0) : Color(50, 100, 220);
                }
                break;

            case RightL:
                p_width = 2;
                p_height = 3;
                for (int n = 0; n < 6; ++n) {
                    p_data[n] = (n & 1) == 0 && n < 4 ?
                        Color(0, 0, 0, 0) : Color(100, 50, 220);
                }
                break;

            case LeftZ:
                p_width = 2;
                p_height = 3;
                for (int n = 0; n < 6; ++n) {
                    p_data[n] = (n == 0) || (n == 5) ?
                        Color(0, 0, 0, 0) : Color(100, 250, 20);
                }
                break;

            case RightZ:
                p_width = 2;
                p_height = 3;
                for (int n = 0; n < 6; ++n) {
                    p_data[n] = (n == 1) || (n == 4) ?
                        Color(0, 0, 0, 0) : Color(20, 250, 100);
                }
                break;

            case T:
                p_width = 3;
                p_height = 2;
                for (int n = 0; n < 6; ++n) {
                    p_data[n] = (n == 3) || (n == 5) ?
                        Color(0, 0, 0, 0) : Color(220, 50, 220);
                }
                break;

            default:
                p_type = None;
                p_width = 0;
                p_height = 0;
        }
    }

    void Flip()
    {
        int newwidth = p_height;
        int newheight = p_width;

        Color newdata[6];
        for (int y = 0; y < newheight; ++y) {
            for (int x = 0; x < newwidth; ++x) {
                newdata[x + y * newwidth] =
                    p_data[(newheight - y - 1) + x * p_width];
            }
        }

        p_width = newwidth;
        p_height = newheight;
        for (int n = 0; n < 6; ++n) {
            p_data[n] = newdata[n];
        }
    }

    void Render(GraphicsAPI &api, float xpos, float ypos, float block_size) const
    {
        int cell = 0;
        for (int y = 0; y < p_height; ++y) {
            for (int x = 0; x < p_width; ++x) {
                api.Rectangle(
                    xpos + x * block_size, ypos + y * block_size,
                    block_size - 2, block_size - 2, p_data[cell++]
                );
            }
        }
    }

    FigureType type() const { return p_type; }
    int width() const { return p_width; }
    int height() const { return p_height; }
    Color data(int x, int y) const { return p_data[x + y * p_width]; }

private:
    FigureType p_type;
    int        p_width;
    int        p_height;
    Color      p_data[6];
};


// game class
class Game
{
public:
    Game() :
        p_mouse_x(0),
        p_mouse_y(0),

        p_field_width(10),
        p_field_height(20),
        p_field_margin(50),

        p_figure_x(4),
        p_figure_y(0),

        p_lines(0),
        p_fall_timer(0),
        p_fall_speed(1)
    {
        p_field = new Color[p_field_width * p_field_height];
        for (int cell = 0; cell < p_field_width * p_field_height; ++cell) {
            p_field[cell] = Color(0, 0, 0, 0);
        }

        p_figure.Make(LeftL);
    }

    ~Game()
    {
        delete[] p_field;
    }

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

        // due to multiple input sources move in one direction could be performed only once
        bool move_left = false;
        bool move_right = false;
        bool move_down = false;
        bool drop = false;
        bool flip = false;

        for (size_t ev = 0; ev < input.event_count; ++ev) {
            switch (input.events[ev].type) {
                case INPUT_KEY_DOWN:
                    switch (input.events[ev].keyboard.key) {
                        case KEY_SPACE: drop = true; break;
                        case KEY_UP:    flip = true; break;
                        case KEY_DOWN:  move_down = true; break;
                        case KEY_LEFT:  move_left = true; break;
                        case KEY_RIGHT: move_right = true; break;
                    }
                    break;

                case INPUT_BUTTON_DOWN:
                    switch (input.events[ev].joystick.button) {
                        case JOY_BUTTON_0: flip = true; break;
                        case JOY_BUTTON_2: drop = true; break;
                    }
                    break;

                case INPUT_POV:
                    switch (input.events[ev].joystick.pov.value) {
                        case JOY_DIRECTION_LEFT:  move_left = true; break;
                        case JOY_DIRECTION_RIGHT: move_right = true; break;
                        case JOY_DIRECTION_DOWN:  move_down = true; break;
                        case JOY_DIRECTION_UP:    flip = true; break;
                    }
                    break;
            }
        }

        if (drop) {
            Drop();
        } else {
            if (flip) {
                FlipFigure();
            }

            if (move_left) {
                MoveLeft();
            }

            if (move_right) {
                MoveRight();
            }

            if (move_down) {
                MoveDown();
            }
        }
    }

    void Update(float interval)
    {
        p_fall_timer += p_fall_speed * interval;
        if (p_fall_timer >= 1)  {
            p_fall_timer -= 1;
            MoveDown();
        }
    }

    void RenderGraphics(GraphicsAPI &api, int width, int height)
    {
        api.Clear(Color(20, 40, 205));

        // compute field pixel size
        float block_size = float((height - p_field_margin * 2) / p_field_height);
        float field_x = width / 2 - p_field_width / 2 * block_size;
        float field_y = float(p_field_margin);

        // render field as set of boxes for now
        int cell = 0;
        for (int y = 0; y < p_field_height; ++y) {
            for (int x = 0; x < p_field_width; ++x) {
                // field background
                api.Rectangle(
                    field_x + x * block_size, field_y + y * block_size,
                    block_size - 2, block_size - 2, Color(0, 0, 0, 20)
                );

                // filed cell brick
                api.Rectangle(
                    field_x + x * block_size, field_y + y * block_size,
                    block_size - 2, block_size - 2, p_field[cell++]
                );
            }
        }

        // render figure
        p_figure.Render(
            api,
            field_x + p_figure_x * block_size,
            field_y + p_figure_y * block_size,
            block_size
        );

        // tiny mouse rectangle, just to show mouse following
        api.Rectangle(p_mouse_x - 5, p_mouse_y - 5, 10, 10, Color(255, 255, 255));
    }

private:
    void ChangeFigureForTesting()
    {
        int t = p_figure.type();
        if (t == T) {
            t = Stick;
        } else {
            ++t;
        }
        int oldheight = p_figure.height();
        p_figure.Make(FigureType(t));
        p_figure_y -= p_figure.height() - oldheight;
    }

    void FlipFigure()
    {
        // flipped figure should be checked for collision first
        // very ugly way to do that is check fillped figure for collision
        // and if it collides - flip it back and restore position
        int oldx = p_figure_x;
        int oldy = p_figure_y;
        int oldheight = p_figure.height();

        p_figure.Flip();
        p_figure_y -= p_figure.height() - oldheight;
        if (p_figure_x + p_figure.width() > p_field_width) {
            p_figure_x = p_field_width - p_figure.width();
        }

        // now figure is flipped and its position adjusted, but
        // without accounting collision with current bricks
        // if flipped figure is colliding with something - flip it back and
        // restore position
        if (Collide(p_figure_x, p_figure_y)) {
            p_figure.Flip();
            p_figure.Flip();
            p_figure.Flip();
            p_figure_x = oldx;
            p_figure_y = oldy;
        }
    }

    void MoveLeft()
    {
        if (!Collide(p_figure_x - 1, p_figure_y)) {
            --p_figure_x;
        }
    }

    void MoveRight()
    {
        if (!Collide(p_figure_x + 1, p_figure_y)) {
            ++p_figure_x;
        }
    }

    void MoveDown()
    {
        if (Collide(p_figure_x, p_figure_y + 1)) {
            PutFigureInTheWall();
        } else {
            ++p_figure_y;
        }
    }

    // drops down current figure
    void Drop()
    {
        while (!Collide(p_figure_x, p_figure_y + 1)) {
            ++p_figure_y;
        }
        PutFigureInTheWall();
    }

    // "copy" current figure bricks into field and set new figure
    // check for full filled rows and remove them
    void PutFigureInTheWall()
    {
        // if figure put outside field top - this is game over
        if (p_figure_y < 0) {
            // now just clear field and reset speed and lines counter
            for (int cell = 0; cell < p_field_width * p_field_height; ++cell) {
                p_field[cell] = Color(0, 0, 0, 0);
            }

            p_fall_speed = 1;
            p_fall_timer = 0;
            p_lines = 0;
        } else {
            // copy figure bricks to field
            for (int y = 0; y < p_figure.height(); ++y) {
                for (int x = 0; x < p_figure.width(); ++x) {
                    Color figurecol = p_figure.data(x, y);
                    if (figurecol.a > 0) {
                        int fieldcell = (p_figure_x + x) + (p_figure_y + y) * p_field_width;
                        p_field[fieldcell] = figurecol;
                    }
                }
            }

            // check wall for destruction of full rows
            // just walk all rows!
            for (int y = 0; y < p_field_height; ++y) {
                int blocksinarow = 0;
                for (int x = 0; x < p_field_width; ++x) {
                    if (p_field[x + y * p_field_width].a > 0) {
                        ++blocksinarow;
                    }
                }

                if (blocksinarow == p_field_width) {
                    // this row should be removed, just do ugly copy of all previous rows
                    for (int yy = y; yy > 0; --yy) {
                        for (int x = 0; x < p_field_width; ++x) {
                            p_field[x + yy * p_field_width] = p_field[x + (yy - 1) * p_field_width];
                        }
                    }
                    for (int x = 0; x < p_field_width; ++x) {
                        p_field[x] = Color(0, 0, 0, 0);
                    }

                    ++p_lines;
                    p_fall_speed += 0.1f;
                }
            }
        }

        // generate new figure
        p_figure.Make(FigureType(rand() % 7 + 1));
        p_figure_x = 4;
        p_figure_y = -p_figure.height();
    }

    // this function checks current figure collision at position posx and posy
    bool Collide(int posx, int posy)
    {
        // game field width boundaries check
        if (posx < 0 || posx + p_figure.width() > p_field_width) {
            return true;
        }

        // game field bottom boundary check
        if (posy + p_figure.height() > p_field_height) {
            return true;
        }

        // check collision with bricks in the game field
        for (int y = 0; y < p_figure.height(); ++y) {
            for (int x = 0; x < p_figure.width(); ++x) {
                int fieldcell = (posx + x) + (posy + y) * p_field_width;
                if (fieldcell >= 0 && p_figure.data(x, y).a > 0 && p_field[fieldcell].a > 0) {
                    return true;
                }
            }
        }

        return false;
    }

private:
    float  p_mouse_x;
    float  p_mouse_y;

    int    p_field_width;  // in cells
    int    p_field_height; // in cells
    int    p_field_margin; // in pixels
    Color *p_field;

    Figure p_figure;       // current figure
    int    p_figure_x;     // and its position x
    int    p_figure_y;     // and y in cells

    int    p_lines;        // how many row lines "broken"
    float  p_fall_timer;   // current time of falling process
    float  p_fall_speed;   // how fast figure falls down one step
};

// main platform source - contains platform entry point and platform specific
// functions
#include "platform.cpp"
