#ifndef _DRINKERBOT_BALANCE_H
#define _DRINKERBOT_BALANCE_H

#define TARE_SAMPLES        (80*5)
#define DERIVATIVE_SAMPLES  8

class HX711
{
    public:
        int sck_pin, dt_pin;
        float zero;

        HX711(int sck_pin_, int dt_pin_);

        // Low-level reading
        bool dataAvailable();
        void init();
        int readBit();
        int read();

        int timePos;
        bool taring;
        int tarePos;
};

#endif
