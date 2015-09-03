#include "signals.h"

#include <stdbool.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/pwm.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>

// Enables dimmable PWM output on the LEDs
#define PWM_OUTPUT

// Configures the PWM generator to the right values
#define PWM_GEN_CONFIGURE(_gen) do {\
    MAP_PWMGenConfigure(PWM1_BASE, (_gen), PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_UP_DOWN |\
        PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_GEN_NO_SYNC | PWM_GEN_MODE_FAULT_UNLATCHED);\
    MAP_PWMGenPeriodSet(PWM1_BASE, (_gen), 0xFFFFU);\
    MAP_PWMGenEnable(PWM1_BASE, (_gen));\
} while (0)

void initializeSignals(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
#ifdef PWM_OUTPUT
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    MAP_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    MAP_GPIOPinConfigure(GPIO_PF1_M1PWM5);
    MAP_GPIOPinConfigure(GPIO_PF2_M1PWM6);
    MAP_GPIOPinConfigure(GPIO_PF3_M1PWM7);
    // Do not use with TM4C123 devices (datasheet p. 420)
    //PWMClockSet(PWM1_BASE, PWM_SYSCLK_DIV_1);
    // Need to set up Generator 2 (M1PWM5=R) and Generator 3 (M1PWM6=B, M1PWM7=G)
    PWM_GEN_CONFIGURE(PWM_GEN_2);
    PWM_GEN_CONFIGURE(PWM_GEN_3);
    MAP_PWMOutputInvert(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, false);
    MAP_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
#else
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
#endif
    signalOff();
    // Button setup (PF4 = SW1, PF0 = SW2)
    // PF0 is locked by default! Why, TI, did you put the button on this pin?
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    MAP_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
    // The buttons have the lowest priority interrupt since the ISR is blank!
    MAP_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_INT_PIN_4, GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    MAP_IntPrioritySet(INT_GPIOF, 0xE0);
    MAP_GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    MAP_IntEnable(INT_GPIOF);
}

// The rest of these functions should be self explanatory

#ifdef PWM_OUTPUT
void signalRed(const uint32_t value)
{
    MAP_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, value);
}

void signalGreen(const uint32_t value)
{
    MAP_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, value);
}

void signalBlue(const uint32_t value)
{
    MAP_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, value);
}
#endif

void signalOff(void)
{
#ifdef PWM_OUTPUT
    signalRed(0U);
    signalGreen(0U);
    signalBlue(0U);
#else
	MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0U);
#endif
}

void signalSuccess(void)
{
#ifdef PWM_OUTPUT
    signalRed(0U);
    signalGreen(1024U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_3);
#endif
}

void signalError(void)
{
#ifdef PWM_OUTPUT
    signalRed(4096U);
    signalGreen(0U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_1);
#endif
}

void signalFaultInterrupt(void)
{
#ifdef PWM_OUTPUT
    signalRed(4096U);
    signalGreen(1024U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_3);
#endif
}

void signalHeartbeatOn(void)
{
#ifdef PWM_OUTPUT
    signalGreen(1024U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
#endif
}

void signalHeartbeatOff(void)
{
#ifdef PWM_OUTPUT
    signalGreen(0U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
#endif
}

void signalI2CDataRequested(void)
{
#ifdef PWM_OUTPUT
    // Only goes to 0xFFFD?
    signalBlue(4096);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
#endif
}

void clearI2CDataRequested(void)
{
#ifdef PWM_OUTPUT
    signalBlue(0U);
#else
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
#endif
}

bool isUserButton1(void)
{
    // Active LOW, weak pull up
    return MAP_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0;
}

void PortFHandler(void)
{
    // This ISR does nothing but clear the interrupt
    // It exists only to wake up the MCU from sleep on button push
    MAP_GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
}
