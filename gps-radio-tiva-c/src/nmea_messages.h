#pragma once

#include "uart.h"

typedef enum LATITUDE_HEMISPHERE_t
{
    LATH_UNKNOWN,
    LATH_NORTH,
    LATH_SOUTH,
} LATITUDE_HEMISPHERE;

typedef enum LONGITUDE_HEMISPHERE_t
{
    LONH_UNKNOWN,
    LONH_EAST,
    LONH_WEST,
} LONGITUDE_HEMISPHERE;

typedef enum GPS_FIX_TYPE_t
{
    GPSFT_INVALID            = 0,
    GPSFT_GPS                = 1, // normal GPS
    GPSFT_DGPS               = 2, // differential GPS, high precision
    GPSFT_PPS                = 3,
    GPSFT_REALTIME_KINEMATIC = 4,
    GPSFT_FLOAT_RTK          = 5,
    GPSFT_ESTIMATED          = 6,
    GPSFT_MANUAL_INPUT_MODE  = 7,
    GPSFT_SIMULATION_MODE    = 8,
} GPS_FIX_TYPE;

typedef struct GpsTime_t
{
    bool isValid;
    uint8_t hours;
    uint8_t minutes;
    float seconds;
} GpsTime;

/*
 * had to choose to use floats as it seems that nmea messages precision can vary given number of sattelites? settings?
 */
typedef struct GpsData_t
{
    bool isValid;
    GpsTime utcTime;
    float latitudeDegrees;
    LATITUDE_HEMISPHERE latitudeHemisphere;
    float longitudeDegrees;
    LONGITUDE_HEMISPHERE longitudeHemisphere;
    float altitudeMslMeters;
    GPS_FIX_TYPE fixType;
    uint8_t numberOfSattelitesInUse;
    float trueCourseDegrees;
    float speedKnots;
    float speedKph;
} GpsData;

int32_t floatLatToInt32(float lat);
int32_t floatLonToInt32(float lon);

void parseGpggaMessageIfValid(Message* pGpggaMessage, GpsData* pResult);

void parseGpvtgMessageIfValid(Message* pGpvtgMessage, GpsData* pResult);

#ifdef UNIT_TEST

    uint32_t findDivider(Message* pGpggaMessage, uint32_t startIdx);

    float parseFloat(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);
    bool parseUInt8(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, uint8_t* pResult);

    void parseGpsTime(GpsTime* pTime, Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);

    LATITUDE_HEMISPHERE parseLatitudeHemisphere(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);
    LONGITUDE_HEMISPHERE parseLongitudeHemisphere(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);

#endif
