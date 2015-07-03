#pragma once

#include "uart.h"

typedef struct GpsData_t
{
    bool isValid;
    int32_t utcTime;
    int32_t latitude;
    int32_t longitude;
    uint32_t altitudeMeters;
    uint32_t trueCourseDegrees;
    uint32_t speedKmh;
    uint8_t numberOfSatellites;
    uint8_t mode;
    uint8_t gpsQualityIndicator;
} GpsData;

void parseGpggaMessageIfValid(const Message* pGpggaMessage, GpsData* pResult);

void parseGpvtgMessageIfValid(const Message* pGpvtgMessage, GpsData* pResult);
