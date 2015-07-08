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
    // Putting FLASH and SRAM into low power mode increases wake up time from 0 to 100 us for
    // a measly gain of 800 uA power saved. Until we make other power savings this is totally
    // not worth it.
#if 0
    SysCtlSleepPowerSet(SYSCTL_FLASH_LOW_POWER | SYSCTL_SRAM_LOW_POWER);
#endif
}
