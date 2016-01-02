#ifndef _DAC_H
#define _DAC_H

class DAC
{
    public:
        DAC(int cs_, int spiNum, int latchPin_);

        void set(int value);
        void latch();
        HardwareSPI spi;

        int latchPin;
        int cs;
};

#endif
