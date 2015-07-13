#include "signals.h"

#include <stdbool.h>

void MyFaultISR()
{
    signalFaultInterrupt();
    while (true)
    {
    }
}
