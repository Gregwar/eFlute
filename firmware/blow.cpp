#include <stdlib.h>
#include <terminal.h>
#include "HX711.h"

#define SAMPLES 100
#define BLOW_PIN 11
static int winSize, winMin, winMax;
static int offset;
static int sample;

void blow_init()
{
    pinMode(BLOW_PIN, INPUT_FLOATING);
    winSize = 0;

    offset = 0;
    for (int k=0; k<SAMPLES; k++) {
        offset += analogRead(BLOW_PIN);
    }
    winMin = winMax = analogRead(BLOW_PIN);
    winSize = 1;
}

bool blow_tick()
{
    static int divider = 0;
    static int tmpSample = 0;

    tmpSample += analogRead(BLOW_PIN);
    divider++;

    if (divider > SAMPLES) {
        sample = tmpSample;
        if (sample > winMax) winMax = sample;
        if (sample < winMin) winMin = sample;
        winSize++;

        if ((winMax-winMin) > 700) {
            winSize = 0;
            winMin = winMax = sample;
        }

        int nOffset = (winMin+winMax)/2;
        int delta = abs(nOffset-offset);
        if (winSize > (10+delta/2)) {
            offset = nOffset;
            winSize = 0;
            winMin = winMax = sample;
        }

        divider = 0;
        tmpSample = 0;
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
