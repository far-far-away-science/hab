#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct Telemetry_t
{
    uint32_t voltage;
    uint32_t cpuTemperature;
} Telemetry;

void initializeTelemetry(void);

void getTelemetry(Telemetry* pTelemetry);
