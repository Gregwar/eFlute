#include <stdlib.h>
#include <wirish/wirish.h>
#include <terminal.h>
#include "Balance.h"

#define VALUE_SIGN(value, length) \
    ((value < (1<<(length-1))) ? \
     (value) \
     : (value-(1<<length)))

Balance::Balance(int sck_pin_, int dt_pin_)
    : sck_pin(sck_pin_), dt_pin(dt_pin_)
{
    weight = 0;
    derivative = 0;
    timePos = 0;
    taring = false;
}

void Balance::init()
{
    zero = 0;

    // Configuring I/Os
    pinMode(sck_pin, OUTPUT);
    digitalWrite(sck_pin, LOW);
    pinMode(dt_pin, INPUT_PULLDOWN);
}

bool Balance::dataAvailable()
{
    return digitalRead(dt_pin)==LOW;
}

int Balance::readBit()
{
    unsigned int result = 0;
    digitalWrite(sck_pin, HIGH);
    delay_us(1);
    for (int k=0; k<50; k++) {
        result += digitalRead(dt_pin);
    }
    result = (result > 25);
    digitalWrite(sck_pin, LOW);
    delay_us(1);

    return result;
}

int Balance::read()
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

void Balance::tare()
{
    zero = 0;
    tarePos = 0;
    taring = true;
}

float Balance::sample(int samples)
{
    float result = 0;
    for (int i=0; i<samples; i++) {
        result += (read()-zero)/100.0;
    }
    return result/samples;
    /*
       float s = 0;
       for (int i=0; i<8; i++) {
       s += (read()-zero)/100.0;
       }
       return s/8;
       */
}

float Balance::doWeight(int samples)
{
    float x = sample(samples);
    return x;
}

void Balance::stabilize()
{
    derivative = 0;
    weight = doWeight(1);
    float delta;

    do {
        delta = 0;
        for (int i = 0; i<5; i++) {
            float last_weight = weight;
            tick(true);
            delta += fabs(weight-last_weight);
        }
    } while (delta > 0.5);
    void stabilize();
}

int compare (const void * a, const void * b)
{
    float fa = *(const float*) a;
    float fb = *(const float*) b;
    return (fa > fb) - (fa < fb);
}

void Balance::tick(bool force)
{
    if (taring) {
        if (dataAvailable()) {
            float value = read();
            tareSamples[tarePos++] = value;
            if (tarePos >= TARE_SAMPLES) {
                qsort(tareSamples, TARE_SAMPLES, sizeof(float), compare);
                zero = tareSamples[TARE_SAMPLES/2];
                taring = false;
            }
        }
    } else {
        if (dataAvailable() || force) {
            // Getting a new sample
            float new_sample = doWeight(1);

            // Discounting the weight
            weight = 0.93*weight+0.07*new_sample;

            // Appending it to the derivatives
            float before = weightWindow[timePos];
            int beforeT = timeWindow[timePos];
            int now = micros();
            weightWindow[timePos] = weight;
            timeWindow[timePos] = now;
            timePos = (timePos+1)%DERIVATIVE_SAMPLES;
            float dt = (now-beforeT)/(float)1000000;
            float dw = (weight-before);
            float der = dw/dt;
            derivative = 0.94*derivative+0.06*der;
        }
    }
}
