#pragma once

void initializeAprsHardware(uint32_t pwmPeriod, uint32_t pwmInitialValue) {}

void enableHx1(void) {}
void disableHx1(void) {}
void enableAprsPwm(void) {}
void disableAprsPwm(void) {}

void clearAprsPwmInterrupt() {}
void setAprsPwmPulseWidth(uint32_t pulseWidth) {}
