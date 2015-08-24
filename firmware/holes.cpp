#include <terminal.h>
#include "holes.h"

static int holes[8];
static int holes_low[8];
static int holes_pin[8] = {HOLE1, HOLE2, HOLE3, HOLE4, HOLE5, HOLE6, HOLE7, HOLE8};

TERMINAL_COMMAND(holes, "Holes")
{
    /*
    digitalWrite(HOLES_EN, LOW);
    delay(1);
    terminal_io()->println(analogRead(HOLE1));
    digitalWrite(HOLES_EN, HIGH);
    delay(1);
    terminal_io()->println(analogRead(HOLE1));
    */

    for (int k=0; k<8; k++) {
        terminal_io()->println(holes[k]);
    }
    terminal_io()->println((int)holes_value());
}

void holes_init()
{
    pinMode(HOLES_EN, OUTPUT);
    pinMode(HOLE1, INPUT_FLOATING);
    pinMode(HOLE2, INPUT_FLOATING);
    pinMode(HOLE3, INPUT_FLOATING);
    pinMode(HOLE4, INPUT_FLOATING);
    pinMode(HOLE5, INPUT_FLOATING);
    pinMode(HOLE6, INPUT_FLOATING);
    pinMode(HOLE7, INPUT_FLOATING);
    pinMode(HOLE8, INPUT_FLOATING);
}

void holes_tick()
{
    static int k = 0;
    static int i = 0;

    if (k == 0) {
        holes_low[i] = analogRead(holes_pin[i]);
        i++;

        if (i >= 8) {
            digitalWrite(HOLES_EN, HIGH);
            k = 1;
            i = 0;
        }
    } else {
        holes[i] = holes_low[i]-analogRead(holes_pin[i]);
        i++;

        if (i >= 8) {
            digitalWrite(HOLES_EN, LOW);
            k = 0;
            i = 0;
        }
    }
}

char holes_value()
{
    char val = 0;

    for (int k=0; k<8; k++) {
        val |= (holes[k] > 1600)<<k;
    }

    return val;
}
