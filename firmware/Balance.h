#ifndef _DRINKERBOT_BALANCE_H
#define _DRINKERBOT_BALANCE_H

#define TARE_SAMPLES        (80*5)
#define DERIVATIVE_SAMPLES  8

class Balance
{
    public:
        int sck_pin, dt_pin;
        float zero;

        Balance(int sck_pin_, int dt_pin_);

        // Low-level reading
        bool dataAvailable();
        void init();
        int readBit();
        int read();

        // Tare the balance
        void tare();

        // Sample and weight
        float sample(int samples = 1);
        float doWeight(int samples = 4);

        // Waits for the balance to be stable
        void stabilize();

        // Do the computation
        void tick(bool force=false);

        float weight;
        float derivative;
        float tareSamples[TARE_SAMPLES];
        float weightWindow[DERIVATIVE_SAMPLES];
        int timeWindow[DERIVATIVE_SAMPLES];
        int timePos;
        bool taring;
        int tarePos;
};

#endif
