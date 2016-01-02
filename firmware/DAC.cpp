#include <wirish/wirish.h>
#include "DAC.h"

DAC::DAC(int cs_, int spiNum, int latchPin_)
    : spi(spiNum), latchPin(latchPin_), cs(cs_)
{
    pinMode(latchPin, OUTPUT);
    digitalWrite(latchPin, HIGH);
    spi.begin(SPI_18MHZ, MSBFIRST, 0);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

void DAC::set(int value)
{
    value |= (1<<12);
    // value |= (1<<13);
    digitalWrite(cs, LOW);
    spi.send((value>>8)&0xff);
    spi.send((value>>0)&0xff);
    digitalWrite(cs, HIGH);
}

void DAC::latch()
{
    digitalWrite(latchPin, LOW);
    digitalWrite(latchPin, HIGH);
}

