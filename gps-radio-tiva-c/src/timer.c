#include "timer.h"
#include "signals.h"

#include <stdbool.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
#include <driverlib/watchdog.h>
#include <driverlib/rom_map.h>

uint32_t timerSeconds = 0;

void initializeTimer(void)
{
    timerSeconds = 0;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    // invoke timer once a second
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, MAP_SysCtlClockGet());
    MAP_IntEnable(INT_TIMER0A);
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);
    // Prepare the watchdog, use WDG0 as WDG1 was locking up
    MAP_WatchdogUnlock(WATCHDOG0_BASE);
    MAP_WatchdogReloadSet(WATCHDOG0_BASE, WATCHDOG_RELOAD);
    MAP_WatchdogResetEnable(WATCHDOG0_BASE);
    MAP_IntEnable(INT_WATCHDOG);
}

uint32_t getSecondsSinceStart(void)
{
    return timerSeconds;
}

void startWatchdog(void)
{
    MAP_WatchdogEnable(WATCHDOG0_BASE);
}

void Timer0IntHandler(void)
{
    ++timerSeconds;
    MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void WatchdogHandler(void)
{
    // If a fault interrupt is running, it has higher priority and will block this IRQ from
    // being serviced
    MAP_WatchdogIntClear(WATCHDOG0_BASE);
}
