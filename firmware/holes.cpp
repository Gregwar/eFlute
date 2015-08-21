#include <terminal.h>
#include "holes.h"

TERMINAL_COMMAND(holes, "Holes")
{
    digitalWrite(HOLES_EN, HIGH);
    terminal_io()->println(analogRead(HOLE1));
    terminal_io()->println(analogRead(HOLE2));
    terminal_io()->println(analogRead(HOLE3));
    terminal_io()->println(analogRead(HOLE4));
    terminal_io()->println(analogRead(HOLE5));
    terminal_io()->println(analogRead(HOLE6));
    terminal_io()->println(analogRead(HOLE7));
    terminal_io()->println(analogRead(HOLE8));
}

void holes_init()
{
    pinMode(HOLES_EN, OUTPUT);
    digitalWrite(HOLES_EN, LOW);
    pinMode(HOLE1, INPUT_FLOATING);
    pinMode(HOLE2, INPUT_FLOATING);
    pinMode(HOLE3, INPUT_FLOATING);
    pinMode(HOLE4, INPUT_FLOATING);
    pinMode(HOLE5, INPUT_FLOATING);
    pinMode(HOLE6, INPUT_FLOATING);
    pinMode(HOLE7, INPUT_FLOATING);
    pinMode(HOLE8, INPUT_FLOATING);
}
