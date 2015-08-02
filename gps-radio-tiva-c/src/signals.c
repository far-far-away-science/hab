#include "signals.h"

#include <stdbool.h>

#include <inc/hw_memmap.h>

#include <driverlib/pwm.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>

// Enables dimmable PWM output on the LEDs
#define PWM_OUTPUT

// Configures the PWM generator to the right values
#define PWM_GEN_CONFIGURE(_gen) do {\
    ROM_PWMGenConfigure(PWM1_BASE, (_gen), PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_UP_DOWN |\
        PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_GEN_NO_SYNC);\
    ROM_PWMGenPeriodSet(PWM1_BASE, (_gen), 0xFFFFU);\
    ROM_PWMGenEnable(PWM1_BASE, (_gen));\
} while (0)

void initializeSignals(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
#ifdef PWM_OUTPUT
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);
    ROM_GPIOPinConfigure(GPIO_PF2_M1PWM6);
    ROM_GPIOPinConfigure(GPIO_PF3_M1PWM7);
    // Do not use with TM4C123 devices (datasheet p. 420)
    //PWMClockSet(PWM1_BASE, PWM_SYSCLK_DIV_1);
    // Need to set up Generator 2 (M1PWM5=R) and Generator 3 (M1PWM6=B, M1PWM7=G)
    PWM_GEN_CONFIGURE(PWM_GEN_2);
    PWM_GEN_CONFIGURE(PWM_GEN_3);
    ROM_PWMOutputInvert(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, false);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
#else
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
#endif
    signalOff();
}

// The rest of these functions should be self explanatory

#ifdef PWM_OUTPUT
void signalRed(const uint32_t value)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, value);
}

void signalGreen(const uint32_t value)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, value);
}

void signalBlue(const uint32_t value)
{
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, value);
}
#endif

void signalOff(void)
{
#ifdef PWM_OUTPUT
    signalRed(0U);
    signalGreen(0U);
    signalBlue(0U);
#else
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0U);
#endif
}

void signalSuccess(void)
{
#ifdef PWM_OUTPUT
    signalRed(0U);
    signalGreen(1024U);
#else
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_3);
#endif
}

void signalError(void)
{
#ifdef PWM_OUTPUT
    signalRed(4096U);
    signalGreen(0U);
#else
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_1);
#endif
}

void signalFaultInterrupt(void)
{
#ifdef PWM_OUTPUT
    signalRed(4096U);
    signalGreen(1024U);
#else
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_3);
#endif
}

void signalI2CDataRequested()
{
#ifdef PWM_OUTPUT
    signalBlue(0xFFFFU);
#else
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
#endif
}

void clearI2CDataRequested()
{
#ifdef PWM_OUTPUT
    signalBlue(0U);
#else
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
#endif
}
