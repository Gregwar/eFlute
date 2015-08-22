#include <stdlib.h>
#include <wirish/wirish.h>
#include <servos.h>
#include <terminal.h>
#include <main.h>
#include <SdFat.h>
#include "holes.h"
#include "blow.h"
#include "samples.h"
#include "dyn.h"

// #define DEBUG

class DAC
{
    public:
        spi_dev *dev;

        DAC(int cs_, int spiNum, int latchPin_)
            : spi(spiNum), latchPin(latchPin_), cs(cs_)
        {
            pinMode(latchPin, OUTPUT);
            digitalWrite(latchPin, HIGH);
            spi.begin(SPI_18MHZ, MSBFIRST, 0);
            pinMode(cs, OUTPUT);
            digitalWrite(cs, HIGH);
            dev = spi.c_dev();
        }

        void set(int value)
        {
            value |= (1<<12);
            // value |= (1<<13);
            digitalWrite(cs, LOW);
            char data[2];
            data[0] = (value>>8)&0xff;
            data[1] = (value>>0)&0xff;

            spi_tx_inline(dev, data, 1);
            spi_tx_inline(dev, data+1, 1);
            digitalWrite(cs, HIGH);
        }

        void latch()
        {
            digitalWrite(latchPin, LOW);
            digitalWrite(latchPin, HIGH);
        }

        HardwareSPI spi;
        int latchPin;
        int cs;
};

DAC dac(31, 2, 27);

const struct sample *currentSample;
const struct sample *nextSample;
int pos, vol, targetVol;

void interrupt()
{
    static int k = 0;

    if (currentSample != NULL) {
        dac.latch();
        int val = currentSample->values[pos];

        val = (2000+val/(18000/vol));
#ifdef DEBUG
        terminal_io()->println(val);
#else
        dac.set(val);
#endif
        pos++;

        if (pos >= currentSample->count) {
            pos = 0;
            currentSample = nextSample;
        }

        k++;
        if (k > 5) {
            k = 0;
            if (vol < targetVol) vol++;
            if (vol > targetVol) vol--;
        }
    } else {
        pos = 0;
        currentSample = nextSample;
    }
}

/**
 * Setup function
 */
void setup()
{
#ifdef DYN
    dyn_init();
#endif

    // Initializing blow
    blow_init();

    // Holes
    holes_init();

    // Remapping SPI1 for sd card
    afio_remap(AFIO_REMAP_SPI1);

    // Terminal
    terminal_init(&SerialUSB);

    // Ticking @32000
#ifndef DEBUG
    HardwareTimer timer(1);
    timer.pause();
    timer.setPrescaleFactor(1);
    timer.setOverflow(1632);
    timer.setCompare(TIMER_CH1, 1);
    timer.attachCompare1Interrupt(interrupt);
    timer.refresh();
    timer.resume();
#endif
    currentSample = NULL;
    nextSample = NULL;
    pos = 0;
}

/**
 * Loop function
 */
void loop()
{
#ifdef DEBUG
    interrupt();
#endif

    static int k = 0;
    k++;

    if (k > 100) {
        k = 0;
    }

    // Terminal
    terminal_tick();

    if (k == 0) {
        // Ticking holes
        holes_tick();
        char holes = holes_value();
        switch (holes) {
#ifdef DYN
            case 0b11111111:
                nextSample = &dyn[0];
                break;
            case 0b10111111:
                nextSample = &dyn[1];
                break;
            case 0b10011111:
                nextSample = &dyn[2];
                break;
            case 0b10001111:
                nextSample = &dyn[3];
                break;
            case 0b10000111:
                nextSample = &dyn[4];
                break;
            case 0b10000011:
                nextSample = &dyn[5];
                break;
            case 0b10000001:
                nextSample = &dyn[6];
                break;
            case 0b10000010:
                nextSample = &dyn[7];
                break;
#else
            case 0b11111111:
                nextSample = &sample_c;
                break;
            case 0b10111111:
                nextSample = &sample_d;
                break;
            case 0b10011111:
                nextSample = &sample_e;
                break;
            case 0b10001111:
                nextSample = &sample_f;
                break;
            case 0b10000111:
                nextSample = &sample_g;
                break;
            case 0b10000011:
                nextSample = &sample_a;
                break;
            case 0b10000001:
                nextSample = &sample_b;
                break;
            case 0b10000010:
                nextSample = &sample_c2;
                break;
#endif
            default:
                nextSample = NULL;
                break;
        }
    }

    // Adjusting volume
    if (blow_tick()) {
        int tmp = (blow_value()/100)-20;
        tmp *= 4;
        if (tmp < 0) tmp = 0;
        if (tmp > 1000) tmp = 1000;
        targetVol = tmp;
    }
}


TERMINAL_COMMAND(benchmark, "Benchmark")
{
    targetVol = 100;
    HardwareTimer timer(1);
    timer.pause();
    nextSample = &sample_c;
    int start = micros();
    for (int k=0; k<44100; k++) {
        interrupt();
    }
    int len = micros()-start;
    terminal_io()->println(len);
    timer.resume();
}
