#include "tiva_c.h"

#include "signals.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/fpu.h>
#include <driverlib/pwm.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom_map.h>
#include <driverlib/pin_map.h>

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

void initializeAprsHardware(uint32_t pwmPeriod, uint32_t pwmInitialValue)
{
    MAP_SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_GPIOPinConfigure(GPIO_PB6_M0PWM0);
    MAP_GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, pwmPeriod);
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pwmInitialValue);
    // Set APRS interrupt to the highest priority. It matters more than most interrupts
    // including I2C and user buttons.
    MAP_IntPrioritySet(INT_PWM0_0, 0x00);
    // GPIO pin used to enable/disable HX1
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_OD);
    MAP_GPIOPinTypeGPIOOutputOD(GPIO_PORTC_BASE, GPIO_PIN_7);
    disableHx1();
}

void enableAprsPwm(void)
{
    MAP_PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
    MAP_PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    MAP_IntEnable(INT_PWM0_0);
    MAP_PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

void disableAprsPwm(void)
{
    MAP_PWMIntDisable(PWM0_BASE, PWM_INT_GEN_0);
    MAP_PWMGenIntTrigDisable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    MAP_IntDisable(INT_PWM0_0);
    MAP_PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
    MAP_PWMGenDisable(PWM0_BASE, PWM_GEN_0);
}

void enableHx1(void)
{
    MAP_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);
    signalHx1Enabled();
}

void disableHx1(void)
{
    MAP_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);
    signalHx1Disabled();
}
