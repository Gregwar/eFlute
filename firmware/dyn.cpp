#include <math.h>
#include <terminal.h>
#include "dyn.h"

#define DATA_SIZE   4000
struct sample dyn[DYN_SIZE];
static short data[DATA_SIZE];

TERMINAL_PARAMETER_FLOAT(freq, "Frequency base", 523.25);
TERMINAL_PARAMETER_FLOAT(h1, "Amplitude for H1", 16000);
TERMINAL_PARAMETER_FLOAT(h2, "Amplitude for H2", 0);
TERMINAL_PARAMETER_FLOAT(h3, "Amplitude for H4", 0);
TERMINAL_PARAMETER_FLOAT(w1, "Wave power", 1);
TERMINAL_PARAMETER_FLOAT(w2, "Wave power", 1);
TERMINAL_PARAMETER_FLOAT(w3, "Wave power", 1);
TERMINAL_PARAMETER_FLOAT(p1, "Phase", 0);
TERMINAL_PARAMETER_FLOAT(p2, "Phase", 0);
TERMINAL_PARAMETER_FLOAT(p3, "Phase", 0);
TERMINAL_PARAMETER_INT(scale, "Scale select", 0);
TERMINAL_PARAMETER_INT(tr, "Transpose", 0);

static float wave(float v, float p)
{
    float sign = (v < 0) ? -1 : 1;
    return sign*pow(fabs(v), p);
}

static const float scale0[] = {0, 2, 4, 5, 7, 9, 11};
#define scale0_size (sizeof(scale0)/sizeof(float))
static const float scale1[] = {0, 3, 5, 6, 7, 10};
#define scale1_size (sizeof(scale1)/sizeof(float))

void dyn_gen()
{
    const float *tones;
    int tones_size;
    if (scale == 0) {
        tones = scale0;
        tones_size = scale0_size;
    }
    if (scale == 1) {
        tones = scale1;
        tones_size = scale1_size;
    }
    int pos = 0;
    int trn = tr;

    for (int k=0; k<DYN_SIZE; k++) {
        int octave = 12*(trn/tones_size);
        if (trn < 0) {
            octave -= 12;
        }
        float f = freq*pow(2, (tones[(trn+10*tones_size)%tones_size]+octave)/12.0);
        trn++;
        int samples = 44100.0/f;
        dyn[k].values = &data[pos];
        if (pos > DATA_SIZE) pos = DATA_SIZE;
        dyn[k].count = samples;
        for (int i=0; i<samples; i++) {
            float x = i/(float)samples;
            float pi = M_PI;
            float a = pi*2*x;
            data[pos++] = 
                  h1*wave(sin(a+p1*pi*2), w1)
                + h2*wave(sin(a*2+p2*pi*2), w2)
                + h3*wave(sin(a*4+p3*pi*2), w3)
                ;
            if (pos >= DATA_SIZE) {
                pos = DATA_SIZE-1;
            }
        }
    }
}

void dyn_init()
{
    dyn_gen();
}

TERMINAL_COMMAND(gen, "Dyn gen")
{
    dyn_gen();
}

TERMINAL_COMMAND(dyn, "Dyn debug")
{
    for (int i=0; i<dyn[0].count; i++) {
        terminal_io()->println(dyn[0].values[i]);
    }
}
