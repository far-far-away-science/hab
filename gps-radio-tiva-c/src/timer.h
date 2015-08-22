#pragma once

#include <stdint.h>

// Reload value for the watchdog timer -- it is a 16 MHz clock, and we want ~2s
// 16 MHz * 2 s = 32000000 cycles
#define WATCHDOG_RELOAD 32000000U

void initializeTimer(void);
void startWatchdog(void);

uint32_t getSecondsSinceStart(void);
