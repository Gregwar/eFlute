#pragma once

#include <stdio.h>
#include <string>

class Flute
{
    public:
        class FluteSample
        {
            public:
                unsigned char holes;
                int blow;

                void print();
        };

        Flute(std::string port);
        FluteSample readSample();

        FILE *file;
};
