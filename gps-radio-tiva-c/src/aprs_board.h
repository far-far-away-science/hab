#pragma once

#include "nmea_messages.h"

struct Callsign
{
    uint8_t callsign[7];
    uint8_t ssid;
};

extern const struct Callsign CALLSIGN_SOURCE;
extern const struct Callsign CALLSIGN_DESTINATION;

void initializeAprs(void);
void sendAprsMessage(const struct GpsData* pGpsData);
