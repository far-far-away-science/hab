#include "signals.h"

#include <stdbool.h>

#include <inc/hw_memmap.h>

#include <driverlib/pwm.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

#define PWM_OUTPUT

// Configures the PWM generator to the right values
#define PWM_GEN_CONFIGURE(_gen) do {\
    ROM_PWMGenConfigure(PWM1_BASE, (_gen), PWM_GEN_MODE_SYNC | PWM_GEN_MODE_DOWN |\
        PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_GEN_NO_SYNC);\
    ROM_PWMGenEnable(PWM1_BASE, (_gen));\
    ROM_PWMGenPeriodSet(PWM1_BASE, (_gen), 0xFFFFU);\
} while (0)

void initializeSignals(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
#ifdef PWM_OUTPUT
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    PWMClockSet(PWM1_BASE, PWM_SYSCLK_DIV_1);
    // Need to set up Generator 2 (M1PWM5=R) and Generator 3 (M1PWM6=B, M1PWM7=G)
    PWM_GEN_CONFIGURE(PWM_GEN_2);
    PWM_GEN_CONFIGURE(PWM_GEN_3);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
#else
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
#endif
    signalOff();
}

void signalRed(uint32_t red)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, red & 0xFFFFU);
}

void signalGreen(uint32_t green)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, green & 0xFFFFU);
}

void signalBlue(uint32_t blue)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, blue & 0xFFFFU);
}

void signalOff(void)
{
#ifdef PWM_OUTPUT
    signalRed(0);
    signalGreen(0);
    signalBlue(0);
#else
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0U);
#endif
}

void signalSuccess(void)
{
#ifdef PWM_OUTPUT
    signalRed(0U);
    signalGreen(1024U);
#else
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_3);
#endif
}

void signalError(void)
{
#ifdef PWM_OUTPUT
    signalRed(4096U);
    signalGreen(0U);
#else
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_1);
#endif
}
