#pragma once

#include "uart.h"

typedef struct GpsData_t
{
    bool isValid;
    uint8_t szUtcTime[10 + 1];
    uint8_t szLatitude[9 + 1];
    uint8_t szLongitude[10 + 1];
    uint8_t gpsQualityIndicator;
    uint8_t szNumberOfSattelites[2 + 1];
    uint8_t szAltitudeMeters[7 + 1];
    uint8_t szTrueCourseDegrees[5 + 1];
    uint8_t szSpeedKnots[5 + 1];
    uint8_t mode;
} GpsData;

void parseGpggaMessageIfValid(const Message* pGpggaMessage, GpsData* pResult);

void parseGpvtgMessageIfValid(const Message* pGpvtgMessage, GpsData* pResult);
