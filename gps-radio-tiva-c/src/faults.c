#include "signals.h"

#include <stdbool.h>

void HardFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
        // Watchdog will bark after 2 seconds so we will not actually hang here
    }
}

void MpuFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
        // Watchdog will bark after 2 seconds so we will not actually hang here
    }
}

void BusFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
        // Watchdog will bark after 2 seconds so we will not actually hang here
    }
}

void UsageFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
        // Watchdog will bark after 2 seconds so we will not actually hang here
    }
}
