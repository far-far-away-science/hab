#include "signals.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

void initializeSignals(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void signalOff(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0U);
}

void signalSuccess(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_3);
}

void signalError(void)
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1);
}
