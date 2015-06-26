#include "tiva_c.h"

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/fpu.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>

void initializeTivaC(void)
{
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    ROM_IntMasterEnable();
    FPUEnable();
    ROM_FPULazyStackingEnable();
}
