#include "tiva_c.h"

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/fpu.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom_map.h>

void initializeTivaC(void)
{
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN | SYSCTL_INT_OSC_DIS);
    // Set up interrupts to 8 priority levels (3 bits), the remaining bits are subpriority
    MAP_IntPriorityGroupingSet(3);
    MAP_FPUEnable();
    MAP_FPULazyStackingEnable();
    MAP_IntMasterEnable();
    // Putting FLASH and SRAM into low power mode increases wake up time from 0 to 100 us for
    // a measly gain of 800 uA power saved. Until we make other power savings this is totally
    // not worth it.
#if 0
    MAP_SysCtlSleepPowerSet(SYSCTL_FLASH_LOW_POWER | SYSCTL_SRAM_LOW_POWER);
#endif
}
