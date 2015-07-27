/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// platform independend engine functions

// set key/button state
static void SetKeyOrButtonState(bool down, uint8_t &state)
{
    bool currentstate = state & BUTTON_DOWN;
    state = down ? BUTTON_DOWN : 0;
    if (currentstate != down) {
        state |= BUTTON_CHANGED;
    }
}
