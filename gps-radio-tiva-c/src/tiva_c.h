#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/pwm.h>
#include <driverlib/rom_map.h>

void initializeTivaC(void);

void initializeAprsHardware(uint32_t pwmPeriod, uint32_t pwmInitialValue);
void enableHx1(void);
void disableHx1(void);
void enableAprsPwm(void);
void disableAprsPwm(void);

#define clearAprsPwmInterrupt() MAP_PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO)
#define setAprsPwmPulseWidth(pulseWidth) MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, (pulseWidth))
