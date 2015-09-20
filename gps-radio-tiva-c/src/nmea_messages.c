#include "nmea_messages.h"

#include <math.h>
#include <ctype.h>
#include <stdlib.h>

#define MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pMessage, tokenStartIdx, tokenOneAfterEndIdx) \
    (tokenStartIdx) = (tokenOneAfterEndIdx) + 1; \
    (tokenOneAfterEndIdx) = findDivider((pMessage), (tokenStartIdx)); \
    if ((tokenOneAfterEndIdx) >= (pMessage)->size) { return; };

int32_t floatAngularCoordinateToInt32_DDMMFF(AngularCoordinate lat)
{
    if (lat.isValid)
    {
        int32_t r = lat.degrees;
        r *= 10000;
        r += (int32_t) (100 * lat.minutes * 60);
        return r;
    }
    else
    {
        return 0;
    }
}

uint32_t findDivider(const Message* pGpggaMessage, uint32_t startIdx)
{
    uint32_t i;
    for (i = startIdx; i < pGpggaMessage->size && pGpggaMessage->message[i] != ','; ++i)
    {
    }
    return i;
}

uint32_t minimal(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

// https://en.wikipedia.org/wiki/Floating_point
float parseFloat(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
{
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return NAN;
    }

    uint32_t w = 0;
    uint32_t f = 0;
    uint32_t fDivisor = 1;
    bool pointEncountered = false;

    // for some reason atof/strtof are crashing tiva.

    // max number of digits plus dot is 9
    const uint32_t lastIdxPlusOne = minimal(tokenOneAfterEndIdx, tokenStartIdx + 9);
    for (uint32_t i = tokenStartIdx; i < lastIdxPlusOne; ++i)
    {
        if (pGpggaMessage->message[i] == '.')
        {
            if (pointEncountered)
            {
                return NAN;
            }
            else
            {
                pointEncountered = true;
            }
            continue;
        }
        else if (!isdigit(pGpggaMessage->message[i]))
        {
            if (i == tokenStartIdx)
            {
                return NAN; // not a number!
            }
            break;
        }
        if (pointEncountered)
        {
            f = f * 10 + (pGpggaMessage->message[i] - '0');
            fDivisor *= 10;
        }
        else
        {
            w = w * 10 + (pGpggaMessage->message[i] - '0');
        }
    }

    return (float) w + (float) f / (float) fDivisor;
}

bool parseUInt8(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, uint8_t* pResult)
{
    if (!pResult || !pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return false;
    }

    uint32_t r = 0;

    // max number of digits is 4
    const uint32_t lastIdxPlusOne = minimal(tokenOneAfterEndIdx, tokenStartIdx + 4);
    for (uint32_t i = tokenStartIdx; i < lastIdxPlusOne; ++i)
    {
        if (!isdigit(pGpggaMessage->message[i]))
        {
            if (i == tokenStartIdx)
            {
                return false; // not a number!
            }
            break;
        }
        r = r * 10 + (pGpggaMessage->message[i] - '0');
    }

    if (r > 255)
    {
        *pResult = 0;
        return false;
    }
    else
    {
        *pResult = r;
        return true;
    }
}

void parseLatLong(uint8_t numberOfDigitsInDegrees,
                  const Message* pGpggaMessage,
                  uint32_t tokenStartIdx,
                  uint32_t tokenOneAfterEndIdx,
                  AngularCoordinate* pCoordinate)
{
    if (!pCoordinate)
    {
        return;
    }        

    pCoordinate->isValid = false;
    
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return;
    }

    uint8_t degrees;

    if (!parseUInt8(pGpggaMessage, tokenStartIdx, minimal(tokenOneAfterEndIdx, tokenStartIdx + numberOfDigitsInDegrees), &degrees))
    {
        return;
    }

    float minutes = 0.0f;
    
    if (tokenStartIdx + numberOfDigitsInDegrees < tokenOneAfterEndIdx)
    {
        minutes = parseFloat(pGpggaMessage, tokenStartIdx + numberOfDigitsInDegrees, tokenOneAfterEndIdx);
    }
    
    if (minutes == 0.0f || isnormal(minutes))
    {
        pCoordinate->isValid = true;
        pCoordinate->degrees = degrees;
        pCoordinate->minutes = minutes;
    }
}

void parseGpsTime(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, GpsTime* pTime)
{
    if (!pTime || !pGpggaMessage)
    {
        return;
    }

    pTime->isValid = false;

    if (tokenStartIdx != tokenOneAfterEndIdx && tokenOneAfterEndIdx - tokenStartIdx >= 6)
    {
        uint8_t hours;
        uint8_t minutes;

        if (!parseUInt8(pGpggaMessage, tokenStartIdx, tokenStartIdx + 2, &hours))
        {
            return;
        }
        if (!parseUInt8(pGpggaMessage, tokenStartIdx + 2, tokenStartIdx + 4, &minutes))
        {
            return;
        }
        float seconds = parseFloat(pGpggaMessage, tokenStartIdx + 4, tokenOneAfterEndIdx);
        if (seconds != 0 && !isnormal(seconds))
        {
            return;
        }

        pTime->isValid = true;
        pTime->hours = hours;
        pTime->minutes = minutes;
        pTime->seconds = seconds;
    }
    
    return;
}

LATITUDE_HEMISPHERE parseLatitudeHemisphere(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
{
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return LATH_UNKNOWN;
    }
    switch (pGpggaMessage->message[tokenStartIdx])
    {
        case 'N':
        case 'n':
        {
            return LATH_NORTH;
        }
        case 'S':
        case 's':
        {
            return LATH_SOUTH;
        }
    }
    return LATH_UNKNOWN;
}

LONGITUDE_HEMISPHERE parseLongitudeHemisphere(const Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
{
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return LONH_UNKNOWN;
    }
    switch (pGpggaMessage->message[tokenStartIdx])
    {
        case 'E':
        case 'e':
        {
            return LONH_EAST;
        }
        case 'W':
        case 'w':
        {
            return LONH_WEST;
        }
    }
    return LONH_UNKNOWN;
}

void parseGpggaMessageIfValid(const Message* pGpggaMessage, GpsData* pResult)
{
    if (!pGpggaMessage || !pResult)
    {
        return;
    }

    uint8_t gpsQuality;
    uint8_t numberOfSattelitesInUse;
    
    // header
    uint32_t tokenStartIdx = 0;
    uint32_t tokenOneAfterEndIdx = findDivider(pGpggaMessage, tokenStartIdx);
    if ((tokenOneAfterEndIdx) >= pGpggaMessage->size)
    {
        return;
    }
    // time utc
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    GpsTime utcTime;
    parseGpsTime(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx, &utcTime);
    // latitude
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    AngularCoordinate latitude;
    parseLatLong(2, pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx, &latitude);
    // latitude hemisphere N/S
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const LATITUDE_HEMISPHERE latitudeHemisphereNS = parseLatitudeHemisphere(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // longitude
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    AngularCoordinate longitude;
    parseLatLong(3, pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx, &longitude);
    // longitude hemisphere E/W
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const LONGITUDE_HEMISPHERE longitudeHemisphereEW = parseLongitudeHemisphere(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // gps quality
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    if (!parseUInt8(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx, &gpsQuality))
    {
        return;
    }
    // number of sattelites in use
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    if (!parseUInt8(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx, &numberOfSattelitesInUse))
    {
        return;
    }
    // horizontal dilution of position
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // altitude MSL
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const float altitudeMslMeters = parseFloat(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);

    const GPS_FIX_TYPE fixType = (GPS_FIX_TYPE) gpsQuality;

    if ((fixType == GPSFT_GPS || fixType == GPSFT_DGPS || fixType == GPSFT_MANUAL_INPUT_MODE) && 
        isnormal(latitude.isValid) && 
        isnormal(longitude.isValid))
    {
        pResult->utcTime = utcTime;
        pResult->latitude = latitude;
        pResult->latitudeHemisphere = latitudeHemisphereNS;
        pResult->longitude = longitude;
        pResult->longitudeHemisphere = longitudeHemisphereEW;
        pResult->fixType = fixType;
        pResult->altitudeMslMeters = altitudeMslMeters;
        pResult->numberOfSattelitesInUse = numberOfSattelitesInUse;
    }
}

void parseGpvtgMessageIfValid(const Message* pGpvtgMessage, GpsData* pResult)
{
    // header
    uint32_t tokenStartIdx = 0;
    uint32_t tokenOneAfterEndIdx = findDivider(pGpvtgMessage, tokenStartIdx);
    if ((tokenOneAfterEndIdx) >= pGpvtgMessage->size)
    {
        return;
    }
    // track
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const float trueCourseDegrees = parseFloat(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // track type
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // reserved
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // speed in knots
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const float speedKnots = parseFloat(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // speed unit
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // speed in kph
    const float speedKph = parseFloat(pGpvtgMessage, tokenStartIdx, tokenOneAfterEndIdx);
    
    pResult->trueCourseDegrees = trueCourseDegrees;
    pResult->speedKnots = speedKnots;
    pResult->speedKph = speedKph;
}
