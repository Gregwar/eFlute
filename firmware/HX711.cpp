#include <stdlib.h>
#include <wirish/wirish.h>
#include <terminal.h>
#include "HX711.h"

#define VALUE_SIGN(value, length) \
    ((value < (1<<(length-1))) ? \
     (value) \
     : (value-(1<<length)))

HX711::HX711(int sck_pin_, int dt_pin_)
    : sck_pin(sck_pin_), dt_pin(dt_pin_)
{
    taring = false;
}

void HX711::init()
{
    zero = 0;

    // Configuring I/Os
    pinMode(sck_pin, OUTPUT);
    digitalWrite(sck_pin, LOW);
    pinMode(dt_pin, INPUT_PULLDOWN);
}

bool HX711::dataAvailable()
{
    return digitalRead(dt_pin)==LOW;
}

int HX711::readBit()
{
    unsigned int result = 0;
    digitalWrite(sck_pin, HIGH);
    delay_us(1);
    /*
    for (int k=0; k<50; k++) {
        result += digitalRead(dt_pin);
    }
    result = (result > 25);
    */
    result = digitalRead(dt_pin);
    digitalWrite(sck_pin, LOW);
    delay_us(1);

    return result;
}

int HX711::read()
{
    while (!dataAvailable()) {
        delay_us(1);
    }
    unsigned int result = 0;

    for (unsigned int k=0; k<24; k++) {
        result = (result<<1);
        result |= readBit();
    }
    readBit();
    readBit();

    return VALUE_SIGN(result, 24);
}
