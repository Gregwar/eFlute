#include <math.h>
#include <terminal.h>
#include "dyn.h"

struct sample dyn[8];
static short data[8][150];

void dyn_gen(float freq_base)
{
    float tones[8] = {0, 2, 4, 5, 7, 9, 11, 12};
    for (int k=0; k<8; k++) {
        float freq = freq_base*pow(2, tones[k]/12.0);
        int samples = 44100.0/freq;
        if (samples > 150) samples = 150;
        dyn[k].values = data[k];
        dyn[k].count = samples;
        for (int i=0; i<samples; i++) {
            data[k][i] = 16000*sin(M_PI*2*i/samples);
        }
    }
}

void dyn_init()
{
    dyn_gen(523.25);
}

TERMINAL_COMMAND(gen, "Dyn gen")
{
    if (argc) {
        dyn_gen(atof(argv[0]));
    }
}

TERMINAL_COMMAND(dyn, "Dyn debug")
{
    for (int i=0; i<dyn[0].count; i++) {
        terminal_io()->println(dyn[0].values[i]);
    }
}
