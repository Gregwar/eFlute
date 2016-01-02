#pragma once

#include <jack/jack.h>
#include <jack/midiport.h>
#include <map>
#include <deque>
#include <mutex>

#define START_NOTE      0
#define STOP_NOTE       1
#define SET_INSTRUMENT  2

class Midi
{
    class Event
    {
        public:
            double t;
            int type;
            int note;
            int velocity;
            int instrument;
    };

    public:
        Midi();
        ~Midi();

        void addEvent(Midi::Event event); 
        Midi::Event popEvent();
        bool isOn(int note);
        void startNote(int note, int velocity);
        void stopNote(int note, int velocity);
        void setInstrument(int instrument);

        jack_client_t *client;
        jack_port_t *output_port;
        double t0, t;
        double nextT;
        bool hasNext;
        std::deque<Event> events;

        std::mutex mutex;
        
        int currentNote;
};
