#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include "Midi.h"

using namespace std::chrono;
double get_time()
{   
    system_clock::time_point tp = system_clock::now();
    return duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() / 1000000000.0;
}


Midi *midi;

static int process(jack_nframes_t nframes, void *arg)
{
    void* port_buf = jack_port_get_buffer(midi->output_port, nframes);
    unsigned char* buffer;
    jack_midi_clear_buffer(port_buf);

    midi->mutex.lock();
    for (unsigned int i=0; i<nframes; i++) {
        midi->t += 1/44100.0;

        if (midi->hasNext) {
            if (midi->t > midi->nextT) {
                auto event = midi->popEvent();

                if (event.type == START_NOTE || event.type == STOP_NOTE) {
                    buffer = jack_midi_event_reserve(port_buf, i, 3);
                    buffer[2] = 100;		
                    buffer[1] = event.note;
                    buffer[0] = event.type == START_NOTE ? 0x90 : 0x80;	
                } else if (event.type == SET_INSTRUMENT) {
                    buffer = jack_midi_event_reserve(port_buf, i, 2);
                    buffer[1] = event.instrument;		
                    buffer[0] = 0xC0;
                }

            }
        }
    }
    midi->mutex.unlock();

    return 0;
}

Midi::Midi()
{
    midi = this;

    if((client = jack_client_open("eFlute", JackNullOption, NULL)) == 0) {
        fprintf (stderr, "JACK server not running?\n");
    }
    jack_set_process_callback(client, process, 0);
    output_port = jack_port_register (client, "out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    jack_activate(client);

    t0 = get_time();
    t = 0.0;
    hasNext = false;
    currentNote = 0;
}

Midi::~Midi()
{
    jack_client_close(client);
}

void Midi::addEvent(Midi::Event event)
{
    event.t = get_time() - t0;
    mutex.lock();
    events.push_back(event);
    if (!hasNext) {
        hasNext = true;
        nextT = event.t;
    }
    mutex.unlock();
}

Midi::Event Midi::popEvent()
{
    auto event = events[0];
    events.pop_front();

    if (events.size() == 0) {
        hasNext = false;
    } else {
        nextT = events[0].t;
    }

    return event;
}
        
void Midi::startNote(int note, int velocity)
{
    if (currentNote != note) {
        if (currentNote != 0) {
            stopNote(currentNote, velocity);
        }
        currentNote = note;
        Midi::Event event;
        event.type = START_NOTE;
        event.note = note;
        event.velocity = velocity;
        addEvent(event);
    }
}

void Midi::stopNote(int note, int velocity)
{
    if (note == currentNote)
    {
        std::cout << "STOP NOTE" << std::endl;
        currentNote = false;
        Midi::Event event;
        event.type = STOP_NOTE;
        event.note = note;
        event.velocity = velocity;
        addEvent(event);
    }
}

void Midi::setInstrument(int instrument)
{
    Midi::Event event;
    event.type = SET_INSTRUMENT;
    event.instrument = instrument;
    addEvent(event);
}
