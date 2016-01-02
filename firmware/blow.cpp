#include <stdlib.h>
#include <terminal.h>
#include "HX711.h"

static HX711 blow(22, 21);
static int winSize, winMin, winMax;
static int offset;
static int sample;

void blow_init()
{
    blow.init();
    winSize = 0;

    blow.read();

    int sum = 0;
    for (int k=0; k<10; k++) {
        sum += blow.read();
    }
    offset = sum/10;
}

bool blow_tick()
{
    if (blow.dataAvailable()) {
        sample = blow.read();

        if (sample > winMax) winMax = sample;
        if (sample < winMin) winMin = sample;
        winSize++;

        if ((winMax-winMin) > 700) {
            winSize = 0;
            winMin = winMax = sample;
        }

        int nOffset = (winMin+winMax)/2;
        int delta = abs(nOffset-offset);
        if (winSize > (10+delta/200)) {
            offset = nOffset;
            winSize = 0;
            winMin = winMax = sample;
        }

        return true;
    }
    return false;
}

int blow_value()
{
    return sample-offset;
}

TERMINAL_COMMAND(blow, "Blow test")
{
    while (!SerialUSB.available()) {
        if (blow_tick()) {
            terminal_io()->print(blow_value());
            terminal_io()->print(" ");
            terminal_io()->print(sample);
            terminal_io()->print(" ");
            terminal_io()->print(offset);
            terminal_io()->println();
        }
    }
}
