#include <terminal.h>
#include "holes.h"

static int holes[8];
static int holes_low[8];

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

    if (k == 0) {
        holes_low[0] = analogRead(HOLE1);
        holes_low[1] = analogRead(HOLE2);
        holes_low[2] = analogRead(HOLE3);
        holes_low[3] = analogRead(HOLE4);
        holes_low[4] = analogRead(HOLE5);
        holes_low[5] = analogRead(HOLE6);
        holes_low[6] = analogRead(HOLE7);
        holes_low[7] = analogRead(HOLE8);
        digitalWrite(HOLES_EN, HIGH);
        k = 1;
    } else {
        holes[0] = holes_low[0]-analogRead(HOLE1);
        holes[1] = holes_low[1]-analogRead(HOLE2);
        holes[2] = holes_low[2]- analogRead(HOLE3);
        holes[3] = holes_low[3]-analogRead(HOLE4);
        holes[4] = holes_low[4]-analogRead(HOLE5);
        holes[5] = holes_low[5]-analogRead(HOLE6);
        holes[6] = holes_low[6]-analogRead(HOLE7);
        holes[7] = holes_low[7]-analogRead(HOLE8);
        digitalWrite(HOLES_EN, LOW);
        k = 0;
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
