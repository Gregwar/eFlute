#ifndef _SAMPLES_H
#define _SAMPLES_H

struct sample {
    const short *values;
    int count;
    int vol;
    int pos;
    bool active;
};
extern const struct sample sample_c;
extern const struct sample sample_d;
extern const struct sample sample_e;
extern const struct sample sample_f;
extern const struct sample sample_g;
extern const struct sample sample_a;
extern const struct sample sample_b;
extern const struct sample sample_c2;

#endif
