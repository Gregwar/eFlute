#include <terminal.h>
#include "holes.h"
#include "mux.h"

static int holes[8];
static int holes_low[8];
static const int holes_mux[8]  = {0, 1, 0, 1, 0, 1, 0, 1};
static const int holes_addr[8] = {0, 0, 1, 1, 2, 2, 3, 3};

int holeSample(int hole)
{
    mux_set_addr(holes_addr[hole]);
    return mux_sample(holes_mux[hole]);
}

TERMINAL_COMMAND(holedbg, "Debug")
{
    digitalWrite(HOLES_EN1, HIGH);
    digitalWrite(HOLES_EN2, HIGH);
    while (!SerialUSB.available()) {
        terminal_io()->println(holeSample(7));
    }
    digitalWrite(HOLES_EN1, LOW);
    digitalWrite(HOLES_EN2, LOW);
}

TERMINAL_COMMAND(off, "Off the holes")
{
    digitalWrite(HOLES_EN1, LOW);
    digitalWrite(HOLES_EN2, LOW);
    while (!SerialUSB.available()) {
    }
    digitalWrite(HOLES_EN1, HIGH);
    digitalWrite(HOLES_EN2, HIGH);
}

TERMINAL_COMMAND(holes, "Holes")
{
    /*
    digitalWrite(HOLES_EN1, LOW);
    digitalWrite(HOLES_EN2, LOW);
    delay(10);
    terminal_io()->println(holeSample(0));
    digitalWrite(HOLES_EN1, HIGH);
    digitalWrite(HOLES_EN2, HIGH);
    delay(10);
    terminal_io()->println(holeSample(0));
    */

    for (int k=0; k<8; k++) {
        terminal_io()->println(holes[k]);
    }
    terminal_io()->println((int)holes_value());
}

void holes_init()
{
    pinMode(HOLES_EN1, OUTPUT);
    pinMode(HOLES_EN2, OUTPUT);
}

bool holes_tick()
{
    static int k = 0;
    static int i = 0;

    if (k == 0) {
        // "Power saving" period
        i++;
        if (i >= 16) {
            k = 1;
            i = 0;
        }
    } else if (k == 1) {
        // Sampling low holes
        holes_low[i] = holeSample(i);
        i++;

        if (i >= 8) {
            digitalWrite(HOLES_EN1, HIGH);
            digitalWrite(HOLES_EN2, HIGH);
            delay_us(10);
            k = 2;
            i = 0;
        }
    } else if (k == 2) {
        // Sampling high holes
        holes[i] = holes_low[i]-holeSample(i);
        i++;

        if (i >= 8) {
            digitalWrite(HOLES_EN1, LOW);
            digitalWrite(HOLES_EN2, LOW);
            delay_us(10);
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
        val |= (holes[k] > 500)<<k;
    }

    return val;
}
