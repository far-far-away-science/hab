#include "timer.h"

#include <stdbool.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>

uint32_t timerSeconds;

void initializeTimer(void)
{
    timerSeconds = 0;
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    // invoke timer once a second
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet());
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

uint32_t getSecondsSinceStart(void)
{
    return timerSeconds;
}

void Timer0IntHandler(void)
{
    ++timerSeconds;
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
