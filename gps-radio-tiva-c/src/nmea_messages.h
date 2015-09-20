#pragma once

#include "uart.h"

typedef enum LATITUDE_HEMISPHERE_t
{
    LATH_UNKNOWN = '?',
    LATH_NORTH   = 'N',
    LATH_SOUTH   = 'S',
} LATITUDE_HEMISPHERE;

typedef enum LONGITUDE_HEMISPHERE_t
{
    LONH_UNKNOWN = '?',
    LONH_EAST    = 'E',
    LONH_WEST    = 'W',
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

typedef struct AngularCoordinate_t
{
    bool isValid;
    uint16_t degrees;
    float minutes;
} AngularCoordinate;

/*
 * had to choose to use floats as it seems that nmea messages precision can vary given number of sattelites? settings?
 */
typedef struct GpsData_t
{
    GpsTime utcTime;
    AngularCoordinate latitude;
    LATITUDE_HEMISPHERE latitudeHemisphere;
    AngularCoordinate longitude;
    LONGITUDE_HEMISPHERE longitudeHemisphere;
    float altitudeMslMeters;
    GPS_FIX_TYPE fixType;
    uint8_t numberOfSattelitesInUse;
    float trueCourseDegrees;
    float speedKnots;
    float speedKph;
} GpsData;

// format: DDMMFF
// D - degree digit
// M - whole minute digit
// F - minute fraction digit
int32_t floatAngularCoordinateToInt32_DDMMFF(AngularCoordinate lat);

void parseGpggaMessageIfValid(const Message* pGpggaMessage, GpsData* pResult);

void parseGpvtgMessageIfValid(const Message* pGpvtgMessage, GpsData* pResult);

#ifdef UNIT_TEST

    uint32_t findDivider(const Message* pGpggaMessage, uint32_t startIdx);

    float parseFloat(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);
    bool parseUInt8(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, uint8_t* pResult);

    void parseGpsTime(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, GpsTime* pTime);
    void parseLatLong(uint8_t numberOfDigitsInDegrees,
                      const Message* pGpggaMessage,
                      uint32_t tokenStartIdx,
                      uint32_t tokenOneAfterEndIdx,
                      AngularCoordinate* pCoordinate);

    LATITUDE_HEMISPHERE parseLatitudeHemisphere(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);
    LONGITUDE_HEMISPHERE parseLongitudeHemisphere(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx);

#endif
