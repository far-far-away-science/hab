#include "signals.h"

#include <stdbool.h>

void MyFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
        // Watchdog will bark after 2 seconds so we will not actually hang here
    }
}
