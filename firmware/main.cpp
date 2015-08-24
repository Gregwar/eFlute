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

TERMINAL_PARAMETER_FLOAT(volGain, "Vol gain", 10);

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

struct sample *currentSample;
int vol, targetVol;
int echoVol, echoPos;
bool compute;
int nextVal;

/*
#define TO_PLAY_SIZE    100
short to_play[TO_PLAY_SIZE];
int to_play_pos;
*/

inline short compute_to_play()
{
    int sumVol = 0, sumVal = 0;
    for (int k=0; k<DYN_SIZE; k++) {
        if (dyn[k].vol > 0 || &dyn[k] == currentSample) {
            int dval = dyn[k].values[dyn[k].pos];
            int dvol = dyn[k].vol;
            sumVal += dvol*(2048+(dval*vol)/16000);
            sumVol += dvol;

            dyn[k].pos++;
            if (dyn[k].pos >= dyn[k].count) {
                dyn[k].pos = 0;
            }
            if (&dyn[k] != currentSample) {
                dyn[k].vol -= 1;
            } else {
                if (dyn[k].vol < 1000) {
                    dyn[k].vol += 1;
                }
            }
        }
    }
    int val = 0;
    if (sumVol > 0) {
        val = sumVal/sumVol;
    }
    return val;
}

void setSample(struct sample *sample)
{
    if (sample != currentSample) {
        currentSample = sample;
    }
}

void interrupt()
{
    static int k = 0;

    if (currentSample != NULL) {
        dac.latch();
        int val = compute_to_play();

#ifdef DEBUG
        terminal_io()->println(val);
#else
        dac.set(val);
#endif

        k++;
        if (k > 2) {
            k = 0;
            if (vol < targetVol) vol++;
            if (vol > targetVol) vol--;
        }
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

    for (int k=0; k<DYN_SIZE; k++) {
        dyn[k].pos = 0;
        dyn[k].vol = 0;
    }
}

/**
 * Loop function
 */
void loop()
{
    static int k = 0;

#ifdef DEBUG
    interrupt();
#endif

    // Terminal
    terminal_tick();

    if (k++ > 300) {
        k = 0;
        // Ticking holes
        holes_tick();
        char holes = holes_value();
        switch (holes) {
#ifdef DYN
            case 0b11111111:
                setSample(&dyn[0]);
                break;
            case 0b10111111:
                setSample(&dyn[1]);
                break;
            case 0b10011111:
                setSample(&dyn[2]);
                break;
            case 0b10001111:
                setSample(&dyn[3]);
                break;
            case 0b10000111:
                setSample(&dyn[4]);
                break;
            case 0b10000011:
                setSample(&dyn[5]);
                break;
            case 0b10000001:
                setSample(&dyn[6]);
                break;
            case 0b10000010:
                setSample(&dyn[7]);
                break;
            case 0b00000010:
                setSample(&dyn[8]);
                break;
            case 0b00011111:
                setSample(&dyn[9]);
                break;
#else
            case 0b11111111:
                currentSample = &sample_c;
                break;
            case 0b10111111:
                currentSample = &sample_d;
                break;
            case 0b10011111:
                currentSample = &sample_e;
                break;
            case 0b10001111:
                currentSample = &sample_f;
                break;
            case 0b10000111:
                currentSample = &sample_g;
                break;
            case 0b10000011:
                currentSample = &sample_a;
                break;
            case 0b10000001:
                currentSample = &sample_b;
                break;
            case 0b10000010:
                currentSample = &sample_c2;
                break;
#endif
        }
    }

    // Adjusting volume
    if (blow_tick()) {
        int tmp = (blow_value()/100)-20;
        tmp *= volGain;
        if (tmp < 0) tmp = 0;
        if (tmp > 2000) tmp = 2000;
        targetVol = tmp;
    }
}

TERMINAL_COMMAND(test, "Test")
{
    dyn_gen();
    for (int k=0; k<DYN_SIZE; k++) {
        setSample(&dyn[k]);
        targetVol = 400;
        delay(100);
    }
}


TERMINAL_COMMAND(benchmark, "Benchmark")
{
    targetVol = 100;
    HardwareTimer timer(1);
    timer.pause();
    setSample(&dyn[0]);

    int start = micros();

    for (int k=0; k<44100; k++) {
        interrupt();
    }

    int len = micros()-start;
    terminal_io()->println(len);

    timer.resume();
}
