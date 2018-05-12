#ifndef CONSOLE_H
#define CONSOLE_H
#include "tap.h"

class Console
{public:
    Tap* tap;

    Console(Tap* tap);

    static void* run(void* arg);

};

#endif