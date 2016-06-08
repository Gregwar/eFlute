#include <terminal.h>
#include "holes.h"

static int holes[8];
static int holes_low[8];
static const int holes_pin[8] = {HOLE1, HOLE2, HOLE3, HOLE4, HOLE5, HOLE6, HOLE7, HOLE8};

TERMINAL_COMMAND(holedbg, "Debug")
{
    digitalWrite(HOLES_EN, HIGH);
    while (!SerialUSB.available()) {
        terminal_io()->println(analogRead(HOLE7));
    }
    digitalWrite(HOLES_EN, LOW);
}

TERMINAL_COMMAND(holes, "Holes")
{
    digitalWrite(HOLES_EN, LOW);
    delay(10);
    terminal_io()->println(analogRead(HOLE1));
    digitalWrite(HOLES_EN, HIGH);
    delay(10);
    terminal_io()->println(analogRead(HOLE1));

    for (int k=0; k<8; k++) {
        terminal_io()->println(holes[k]);
    }
    terminal_io()->println((int)holes_value());
}

void holes_init()
{
    pinMode(HOLES_EN, OUTPUT);
    for (int k=0; k<8; k++) {
        pinMode(holes_pin[k], INPUT_FLOATING);
    }
}

bool holes_tick()
{
    static int k = 0;
    static int i = 0;

    if (k == 0) {
        //holes_low[i] = analogRead(holes_pin[i]);
        holes_low[i] = 4095;
        i++;

        if (i >= 8) {
            digitalWrite(HOLES_EN, HIGH);
            // delay_us(10);
            k = 1;
            i = 0;
        }
    } else {
        holes[i] = holes_low[i]-analogRead(holes_pin[i]);
        i++;

        if (i >= 8) {
            // digitalWrite(HOLES_EN, LOW);
            // delay_us(10);
            k = 0;
            i = 0;
        }
    }

    return (k==0 && i==0);
}

void holes_cycle()
{
    while (!holes_tick());
}

char holes_value()
{
    char val = 0;

    for (int k=0; k<8; k++) {
        val |= (holes[k] > 50)<<k;
    }

    return val;
}
