#include <stdlib.h>
#include <wirish/wirish.h>
#include <servos.h>
#include <terminal.h>
#include <main.h>
#include <SdFat.h>
#include "Balance.h"

#define HOLE1       3
#define HOLE2       4
#define HOLE3       5
#define HOLE4       6
#define HOLE5       7
#define HOLE6       8
#define HOLE7       9
#define HOLE8       10
#define HOLES_EN    2

Balance blow(22, 21);

TERMINAL_COMMAND(tare, "Tare")
{
    blow.tare();
    while (blow.taring) {
        blow.tick();
    }
}

TERMINAL_COMMAND(blow, "Blow test")
{
    while (!SerialUSB.available()) {
        terminal_io()->println(blow.sample());
    }
}

// Sign an expression
#define VALUE_SIGN(value, length) \
    ((value < (1<<(length-1))) ? \
    (value) \
    : (value-(1<<length)))

TERMINAL_PARAMETER_INT(vol, "Volume", 100);

bool playing = false;
HardwareSPI spi(1);
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

class DAC
{
    public:
        DAC(int cs_, int spiNum, int latchPin_)
            : spi(spiNum), latchPin(latchPin_), cs(cs_)
        {
            pinMode(latchPin, OUTPUT);
            digitalWrite(latchPin, HIGH);
            spi.begin(SPI_18MHZ, MSBFIRST, 0);
            pinMode(cs, OUTPUT);
            digitalWrite(cs, HIGH);
        }

        void set(int value)
        {
            value |= (1<<12);
            // value |= (1<<13);
            digitalWrite(cs, LOW);
            spi.send((value>>8)&0xff);
            spi.send((value>>0)&0xff);
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

TERMINAL_COMMAND(dac, "dac set")
{
    if (argc != 1) {
        terminal_io()->println("Usage: dac [value]");
    } else {
        int value = atoi(argv[0]);
        dac.set(value);
        dac.latch();
    }
}

#include "sin.h"

TERMINAL_COMMAND(sin, "Sinus")
{
    float amp = atof(argv[0]);
    float samp = 0;
    int i = 0;
    while (!SerialUSB.available()) {
//        samp = 0.995*samp+0.005*amp;
        i++;
        if (i >= 50) i = 0;
        dac.set(values[i]*(samp/2)+2000);
        dac.latch();
        if (blow.dataAvailable()) {
            samp = (blow.read()-blow.zero)/100;
            delay_us(1);
        } else {
            delay_us(6);
        }
    }
    /*
    while (samp > 0.01) {
        samp = 0.995*samp;
        i++;
        if (i >= 50) i = 0;
        dac.set(values[i]*(samp/2)+2000);
        dac.latch();
        delay_us(6);
    }
    */
}

#define SAMPLES_BUF 512
int samples1[SAMPLES_BUF];
int samples2[SAMPLES_BUF];
int samples_buffer = 1;
int samples_position = 0;
bool should_fill = false;

void interrupt()
{
    dac.latch();
    if (samples_position < 0) {
        samples_position = SAMPLES_BUF-1;
        should_fill = true;
        if (samples_buffer == 1) {
            samples_buffer = 2;
        } else {
            samples_buffer = 1;
        }
    }
    if (samples_buffer == 1) {
        dac.set(2000+(samples1[samples_position--]/(16000/vol)));
    } else {
        dac.set(2000+(samples2[samples_position--]/(16000/vol)));
    }
}

void fillBuffer(int *samples)
{
    char buffer[SAMPLES_BUF*2];
    file.read(buffer, SAMPLES_BUF*2);

    for (int k=0; k<SAMPLES_BUF; k++) {
        int sample = (buffer[2*k]&0xff);
        sample |= (buffer[2*k+1]&0xff)<<8;
        sample = VALUE_SIGN(sample, 16);
        samples[SAMPLES_BUF-k-1] = sample;
    }
}

void populateSamples()
{
    if (should_fill) {
        should_fill = false;
        if (samples_buffer == 1) {
            fillBuffer(samples2);
        } else {
            fillBuffer(samples1);
        }
    }
}

TERMINAL_COMMAND(sd, "Sd init")
{
    spi.begin(SPI_18MHZ, MSBFIRST, 0);
    if (card.init(&spi)) {
        terminal_io()->println("Success in card init");
        delay(100);
        if (volume.init(&card,1)) {
            terminal_io()->println("Volume mounted");
            if (root.openRoot(&volume)) {
                terminal_io()->println("Root opened");
            }
        }
    }
}

TERMINAL_COMMAND(play, "Play a sound")
{
    if (argc != 1) {
        terminal_io()->println("Usage: play sound.raw");
    } else {
        playing = false;
        if (file.isOpen()) file.close();
        if (file.open(&root, argv[0], O_READ)) {
            HardwareTimer timer(1);
            timer.pause();
            timer.setPrescaleFactor(1);
            timer.setOverflow(2250);
            timer.setCompare(TIMER_CH1, 1);
            timer.attachCompare1Interrupt(interrupt);
            timer.refresh();
            timer.resume();

            playing = true;

            /*
            for (int k=0; k<64; k++) {
                terminal_io()->println((int)getSample());
            }
            */
        }
    }
}

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

/**
 * Setup function
 */
void setup()
{
    blow.init();

    // For sd card
    afio_remap(AFIO_REMAP_SPI1);

    // Holes
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

    terminal_init(&SerialUSB);
}

/**
 * Loop function
 */
void loop()
{
    terminal_tick();

    if (playing) {
        populateSamples();

        if (blow.dataAvailable()) {
            int tmp = (blow.sample()-24);
            if (tmp < 0) tmp = 0;
            if (tmp > 1000) tmp = 1000;
            vol = 0.8*vol+0.2*tmp;
        }
    }
}
