#include <math.h>
#include <terminal.h>
#include "dyn.h"

struct sample dyn[DYN_SIZE];
static short data[DYN_SIZE][200];

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
TERMINAL_PARAMETER_INT(tr, "Transpose", 0);
TERMINAL_PARAMETER_INT(scale, "Scale select", 0);

static float wave(float v, float p)
{
    float sign = (v < 0) ? -1 : 1;
    return sign*pow(fabs(v), p);
}

static const float scale0[DYN_SIZE] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16};
static const float scale1[DYN_SIZE] = {0, 3, 5, 6, 7, 10, 12, 15, 17, 18};

void dyn_gen()
{
    const float *tones;
    if (scale == 0) tones = scale0;
    if (scale == 1) tones = scale1;

    for (int k=0; k<DYN_SIZE; k++) {
        float f = freq*pow(2, (tones[k]+tr)/12.0);
        int samples = 44100.0/f;
        if (samples > 200) samples = 200;
        dyn[k].values = data[k];
        dyn[k].count = samples;
        for (int i=0; i<samples; i++) {
            float x = i/(float)samples;
            float pi = M_PI;
            float a = pi*2*x;
            data[k][i] = 
                  h1*wave(sin(a+p1*pi*2), w1)
                + h2*wave(sin(a*2+p2*pi*2), w2)
                + h3*wave(sin(a*4+p3*pi*2), w3)
                ;
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
