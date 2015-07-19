/*
    TETRIS FROM SCRATCH
    (C) livingcreative, 2015

    feel free to use and modify
*/

// platform independend engine functions

static InputEvent *new_event(Input &input)
{
    if (input.event_count == INPUT_EVENT_COUNT) {
        return nullptr;
    } else {
        return input.events + input.event_count++;
    }
}
