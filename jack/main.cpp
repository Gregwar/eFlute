#include <signal.h>
#include <iostream>
#include <unistd.h>
#include "Midi.h"
#include "Flute.h"
    
bool terminate = false;

void signal_handler(int sig)
{
    terminate = true;
}

int main()
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    Midi midi;
    Flute flute("/dev/ttyACM0");
    int k = 0;

    while (!terminate) {
        auto sample = flute.readSample();

        if (k++ > 50) {
            k = 0;
            system("clear");
            sample.print();
            midi.setInstrument(74);
        }

        int note = 0;
        switch (sample.holes) {
                case 0b11111111:
                    note = 60;
                    break;
                case 0b10111111:
                    note = 62;
                    break;
                case 0b10011111:
                    note = 64;
                    break;
                case 0b10001111:
                    note = 65;
                    break;
                case 0b10000111:
                    note = 67;
                    break;
                case 0b10000011:
                    note = 69;
                    break;
                case 0b10000001:
                    note = 71;
                    break;
                case 0b10000010:
                    note = 72;
                    break;
                case 0b00000010:
                    note = 74;
                    break;
                case 0b00011111:
                    note = 76;
                    break;
        }

        if (note != 0) {
            note += 12;
            if (sample.blow > 10) {
                midi.startNote(note, 127);
            } else {
                midi.stopNote(note, 127);
            }
        }
    }
}
