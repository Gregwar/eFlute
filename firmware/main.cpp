#include <stdlib.h>
#include <wirish/wirish.h>
#include <servos.h>
#include <terminal.h>
#include <main.h>
#include <SdFat.h>
#include "holes.h"
#include "blow.h"
#include "samples.h"
#include "bt.h"
#include "dyn.h"
#include "mux.h"

HardwareTimer timer(1);
bool isUSB = false;

TERMINAL_COMMAND(rc, "Go to RC mode")
{
    Serial3.begin(115200);
    terminal_init(&Serial3);
    isUSB = false;
}

TERMINAL_COMMAND(forward,
        "Go to forward mode, the Serial3 will be forwarded to USB and vice-versa. Usage: forward [baudrate]")
{
    int baudrate = 115200;
    char buffer[512];
    unsigned int pos;
    terminal_io()->print("The forward mode will be enabled, ");
    terminal_io()->println("you'll need to reboot the board to return to normal operation");

    if (argc) {
        baudrate = atoi(argv[0]);
    }
    terminal_io()->print("Using baudrate ");
    terminal_io()->println(baudrate);

    Serial3.begin(baudrate);

    while (1) {
        pos = 0;
        while (Serial3.available() && pos < sizeof(buffer)) {
            buffer[pos++] = Serial3.read();
        }

        if (pos > 0) {
            SerialUSB.write(buffer, pos);
        }

        while (SerialUSB.available()) {
            Serial3.write(SerialUSB.read());
        }
    }
}

TERMINAL_PARAMETER_BOOL(bin, "Bin mode", false);
TERMINAL_PARAMETER_INT(binDivider, "Bin divider", 0);

TERMINAL_PARAMETER_FLOAT(volGain, "Vol gain", 20.0);

// #define DEBUG

#define DAC_CS          31
#define DAC_SPI         2
#define DAC_LATCH       27

int buttons[6] = {15, 16, 17, 18, 19, 20};

TERMINAL_COMMAND(but, "Buttons")
{
    for (int k=0; k<6; k++) {
        terminal_io()->print("Button ");
        terminal_io()->print(k);
        terminal_io()->print(": ");
        terminal_io()->println(digitalRead(buttons[k]));
    }
}

HardwareSPI dac_spi(DAC_SPI);
spi_dev *dac_dev;

void dac_init()
{
    dac_spi.begin(SPI_18MHZ, MSBFIRST, 0);

    pinMode(DAC_LATCH, OUTPUT);
    digitalWrite(DAC_LATCH, HIGH);

    pinMode(DAC_CS, OUTPUT);
    digitalWrite(DAC_CS, HIGH);
    dac_dev = dac_spi.c_dev();
}

inline void dac_shut()
{
    int value = 0;
    gpio_write_bit(PIN_MAP[DAC_CS].gpio_device, PIN_MAP[DAC_CS].gpio_bit, LOW);
    char data[2];
    data[0] = (value>>8)&0xff;
    data[1] = (value>>0)&0xff;

    spi_tx_inline(dac_dev, data, 1);
    spi_tx_inline(dac_dev, data+1, 1);
    for (int K=0; K<7; K++) {
        asm volatile("nop");
    }
    gpio_write_bit(PIN_MAP[DAC_CS].gpio_device, PIN_MAP[DAC_CS].gpio_bit, HIGH);
}

inline void dac_set(int value)
{
    value |= (1<<12);
    // value |= (1<<13);
    gpio_write_bit(PIN_MAP[DAC_CS].gpio_device, PIN_MAP[DAC_CS].gpio_bit, LOW);
    // digitalWrite(DAC_CS, LOW);
    char data[2];
    data[0] = (value>>8)&0xff;
    data[1] = (value>>0)&0xff;

    spi_tx_inline(dac_dev, data, 1);
    spi_tx_inline(dac_dev, data+1, 1);
    for (int K=0; K<7; K++) {
        asm volatile("nop");
    }
    gpio_write_bit(PIN_MAP[DAC_CS].gpio_device, PIN_MAP[DAC_CS].gpio_bit, HIGH);
    // digitalWrite(DAC_CS, HIGH);
}

inline void dac_latch()
{
    gpio_write_bit(PIN_MAP[DAC_LATCH].gpio_device, PIN_MAP[DAC_LATCH].gpio_bit, LOW);
    gpio_write_bit(PIN_MAP[DAC_LATCH].gpio_device, PIN_MAP[DAC_LATCH].gpio_bit, HIGH);
}

struct sample *currentSample;
struct sample *currentSamples[DYN_SIZE];
int currentSamplesNb;
TERMINAL_PARAMETER_INT(targetVol, "Target vol", 0);
int vol;
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
    for (int k=0; k<currentSamplesNb; k++) {
        int dval = currentSamples[k]->values[currentSamples[k]->pos];
        int dvol = currentSamples[k]->vol;
        if (dvol > 0 || currentSamples[k] == currentSample) {
            sumVal += dvol*((dval*vol)/16000);
            sumVol += dvol;

            currentSamples[k]->pos++;
            if (currentSamples[k]->pos >= currentSamples[k]->count) {
                currentSamples[k]->pos = 0;
            }
            if (currentSamples[k] != currentSample) {
                currentSamples[k]->vol -= 1;
            } else {
                if (currentSamples[k]->vol < 1000) {
                    currentSamples[k]->vol += 1;
                }
            }
        }
    }
    int val = 0;
    if (sumVol > 0) {
        val = sumVal/sumVol;
    }
    return 2048+val;
}

TERMINAL_COMMAND(dbg, "dbg")
{
    terminal_io()->print("There is ");
    terminal_io()->println(currentSamplesNb);
    for (int k=0; k<currentSamplesNb; k++) {
        terminal_io()->print("Sample #");
        terminal_io()->print(k);
        terminal_io()->print(": ");
        terminal_io()->print(currentSamples[k]->vol);
        terminal_io()->println();
    }
}

void setSample(struct sample *sample)
{
    if (sample != currentSample) {
        currentSample = sample;
        // Checking if the sample is already present in the list
        for (int k=0; k<currentSamplesNb; k++) {
            if (currentSamples[k] == sample) {
                return;
            }
        }
        // Adding it
        currentSamples[currentSamplesNb++] = sample;
    }
}

TERMINAL_COMMAND(dac, "Dac debug")
{
    dac_set(atoi(argv[0]));
    dac_latch();
}

void interrupt()
{
    if (bin) {
        binDivider++;
        return;
    } else {
        static int k = 0;

        if (currentSample != NULL && vol > 0) {
            dac_latch();
            int val = compute_to_play();

#ifdef DEBUG
            terminal_io()->println(val);
#else
            dac_set(val);
#endif
        }

        k++;
        if (k > 2) {
            k = 0;
            if (vol < targetVol) vol++;
            if (vol > targetVol) vol--;
        }
    }
}

TERMINAL_COMMAND(vol, "See the vol")
{
    terminal_io()->println(vol);
}

/**
 * Setup function
 */
void setup()
{
    for (int k=0; k<6; k++) {
        pinMode(buttons[k], INPUT_PULLUP);
    }

    // Bluetooth setup
    bt_init();

    // Terminal
    terminal_init(&Serial3);

    // DAC
    dac_init();


#ifdef DYN
    dyn_init();
#endif
    
    // Mux
    mux_init();

    // Initializing blow
    blow_init();

    // Holes
    holes_init();

    // Remapping SPI1 for sd card
    //    afio_remap(AFIO_REMAP_SPI1);

    // Ticking @32000
#ifndef DEBUG
    timer.pause();
    timer.setPrescaleFactor(1);
    timer.setOverflow(1632);
    timer.setCompare(TIMER_CH1, 1);
    timer.attachCompare1Interrupt(interrupt);
    timer.refresh();
    timer.resume();
#endif
    currentSample = NULL;
    currentSamplesNb = 0;

    for (int k=0; k<DYN_SIZE; k++) {
        dyn[k].pos = 0;
        dyn[k].vol = 0;
    }
}

void inhale()
{
    int note = -1;
    for (int k=0; k<DYN_SIZE; k++) {
        if (currentSample == &dyn[k]) note = k;
    }
    /*
       terminal_io()->print("Inhale: ");
       terminal_io()->println(note);
       */

    if (note == 0) {
        set_freq(523.25);
    }
    if (note == 1) {
        set_freq(698.46);
    }
    if (note == 2) {
        set_freq(783.99);
    }

    dyn_gen();
}

int holes_st = 0;
int blows_st = 0;

/**
 * Loop function
 */
void loop()
{
    if (SerialUSB.available() && !isUSB) {
        isUSB = true;
        terminal_init(&SerialUSB);
    }  
    static int k = 0;

#ifdef DEBUG
    interrupt();
#endif

    // Terminal
    terminal_tick();

    if (k++ > 10) {
        k = 0;
        // Ticking holes
        if (holes_tick()) {
            holes_st++;
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
                case 0b01111111:
                    setSample(&dyn[7]);
                    break;
                case 0b00111111:
                    setSample(&dyn[8]);
                    break;
                case 0b00011111:
                    setSample(&dyn[9]);
                    break;
                case 0b00001111:
                    setSample(&dyn[10]);
                    break;
                case 0b00000111:
                    setSample(&dyn[11]);
                    break;
                case 0b00000011:
                    setSample(&dyn[12]);
                    break;
                case 0b00000001:
                    setSample(&dyn[13]);
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
    }

    // Adjusting volume
    if (blow_tick()) {
        blows_st++;
        static bool inhaling = false;
        int tmp = (blow_value()/100)-30;

        if (inhaling) {
            if (tmp > -40) inhaling = false;
        } else {
            if (tmp < -300) {
                inhaling = true;
                inhale();
            }
        }
        tmp *= volGain;
        if (tmp < 0) tmp = 0;
        if (tmp > 2000) tmp = 2000;
        targetVol = tmp;
    }

    // Checking if samples can be removed from the list
    for (int k=0; k<currentSamplesNb; k++) {
        if (currentSamples[k]->vol <= 0 && currentSamples[k] != currentSample) {
            currentSamples[k] = currentSamples[currentSamplesNb-1];
            currentSamplesNb--;
        }
    }

    if (binDivider > 100) {
        char packet[6];
        binDivider -= 44;
        packet[0] = 0xaa;
        packet[1] = 0xaa;
        packet[2] = holes_value();
        int b = blow_value();
        if (b < 0) b = 0;
        b /= 30;
        packet[3] = (b>>8)&0xff;
        packet[4] = (b>>0)&0xff;
        packet[5] = 0x00;
        terminal_io()->write(packet, 6);
    }
}

TERMINAL_COMMAND(test, "Test")
{
    dyn_gen();
    if (argc == 0) {
        targetVol = 400;
    } else {
        targetVol = atoi(argv[0]);
    }
    for (int k=0; k<DYN_SIZE; k++) {
        setSample(&dyn[k]);
        delay(100);
    }
    targetVol = 0;
}

TERMINAL_COMMAND(st, "Stats")
{
    int elapsed = millis();
    float b = blows_st*1000.0/elapsed;
    float h = holes_st*1000.0/elapsed;

    terminal_io()->print("Blows: ");
    terminal_io()->println(b);
    terminal_io()->print("Holes: ");
    terminal_io()->println(h);
}

TERMINAL_COMMAND(benchmark, "Benchmark holes")
{
    int start, len;

    targetVol = 100;
    HardwareTimer timer(1);
    timer.pause();
    setSample(&dyn[0]);

    terminal_io()->println("Doing 1000 holes cycles");
    start = micros();
    for (int k=0; k<1000; k++) {
        holes_cycle();
    }
    len = micros()-start;
    terminal_io()->print("µs: ");
    terminal_io()->println(len);

    terminal_io()->println("Calling interrupt 44100 times");
    start = micros();
    for (int k=0; k<44100; k++) {
        interrupt();
    }
    len = micros()-start;
    terminal_io()->println(len);

    terminal_io()->println("Reading 160 holes");
    len = 0;
    for (int k=0; k<160; k++) {
        start = micros();
        if (blow_tick()) {
            len += micros()-start;
        } else {
            k--;
        }
    }
    terminal_io()->println(len);

    terminal_io()->println("Latching the DAC 44100 times");
    start = micros();
    for (int k=0; k<44100; k++) {
        dac_latch();
    }
    len = micros()-start;
    terminal_io()->println(len);

    timer.resume();
}

TERMINAL_COMMAND(ss, "Small sinus")
{
    targetVol = atoi(argv[0]);
    setSample(&dyn[0]);
    delay(1000);
}
