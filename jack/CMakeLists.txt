cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-reorder -pedantic -fno-elide-constructors")

set (SOURCES
    Flute.cpp
    Midi.cpp
    main.cpp
    )

add_executable (flute ${SOURCES})
target_link_libraries (flute jack)
