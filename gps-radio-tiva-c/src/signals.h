#pragma once

#include <stdint.h>

void initializeSignals(void);
void signalOff(void);
void signalSuccess(void);
void signalError(void);

void signalRed(uint32_t red);
void signalGreen(uint32_t green);
void signalBlue(uint32_t blue);
