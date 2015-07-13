#pragma once

#include <stdint.h>

void initializeSignals(void);
void signalOff(void);

void signalSuccess(void);
void signalError(void);
void signalFaultInterrupt(void);

void signalI2CDataRequested(void);
void clearI2CDataRequested(void);
