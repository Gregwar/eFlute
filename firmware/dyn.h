#ifndef _DYN_H
#define _DYN_H

#include "samples.h"
#define DYN
#define DYN_SIZE        10

void set_freq(float v);
void set_tr(int v);
void set_scale(int v);

extern struct sample dyn[DYN_SIZE];
void dyn_gen();
void dyn_init();

#endif
