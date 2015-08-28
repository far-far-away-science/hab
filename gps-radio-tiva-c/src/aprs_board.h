#pragma once

#include "telemetry.h"
#include "nmea_messages.h"

typedef struct Callsign_t
{
    const uint8_t callsign[7];
    const uint8_t ssid;
} Callsign;

extern const Callsign CALLSIGN_SOURCE;
extern const Callsign CALLSIGN_DESTINATION_1;
extern const Callsign CALLSIGN_DESTINATION_2;

void initializeAprs(void);
bool sendAprsMessage(const GpsData* pGpsData, const Telemetry* pTelemetry);
