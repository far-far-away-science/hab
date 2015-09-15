#pragma once

#include <stdbool.h>
#include <stdint.h>

void initializeSignals(void);
void signalOff(void);

void signalSuccess(void);
void signalError(void);
void signalFaultInterrupt(void);
void signalHeartbeatOn(void);
void signalHeartbeatOff(void);

void signalHx1Enabled(void);
void signalHx1Disabled(void);

// PWM mode only!
void signalRed(const uint32_t value);
void signalGreen(const uint32_t value);
void signalBlue(const uint32_t value);

// User button checking
bool isUserButton1(void);
