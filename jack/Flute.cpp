#include <iostream>
#include "Flute.h"

Flute::Flute(std::string fileName)
{
    file = fopen(fileName.c_str(), "r");
}

Flute::FluteSample Flute::readSample()
{
    Flute::FluteSample sample;
    unsigned char c;
    int state = 0;
    bool over = false;

    while (!over) {
        if (fread(&c, 1, 1, file)) {
            switch (state) {
                case 0:
                case 1:
                    if (c == 0xaa) {
                        state++;
                    } else {
                        state = 0;
                    }
                    break;
                case 2:
                    sample.holes = c;
                    sample.blow = 0;
                    state++;
                    break;
                case 3:
                    sample.blow = (c<<8);
                    state++;
                    break;
                case 4:
                    sample.blow |= c;
                    state++;
                    break;
                case 5:
                    if (c == 0x00) {
                        over = true;
                    }
                    break;
            }
        }
    }

    return sample;
}

void Flute::FluteSample::print()
{
   std::cout << "# FluteSample" << std::endl;
   std::cout << "Holes: ";
   for (int h=0; h<8; h++) {
       if (holes & (1<<h)) {
           std::cout << "[X] ";
       } else {
           std::cout << "[ ] ";
       }
   }
   std::cout << std::endl;
   std::cout << "Blow : [";
   for (int k=0; k<30; k++) {
       if (k*100 < blow) {
           std::cout << "#";
       } else {
           std::cout << " ";
       }
   }
   std::cout << "]" << std::endl;
}
