#include "nmea_messages.h"

#include <math.h>
#include <ctype.h>
#include <stdlib.h>

#define MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pMessage, tokenStartIdx, tokenOneAfterEndIdx) \
    (tokenStartIdx) = (tokenOneAfterEndIdx) + 1; \
    (tokenOneAfterEndIdx) = findDivider((pMessage), (tokenStartIdx)); \
    if ((tokenOneAfterEndIdx) >= (pMessage)->size) { return; };

int32_t floatLatToInt32(float lat)
{
    int32_t r = (int32_t) lat;
    lat -= r;
    r *= 10000;
    r += (int32_t) (100 * lat * 60);
    return r;
}

int32_t floatLonToInt32(float lon)
{
    int32_t r = (int32_t) lon;
    lon -= r;
    r *= 10000;
    r += (int32_t) (100 * lon * 60);
    return r;
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
float parseFloat(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
{
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return NAN;
    }

    uint32_t w = 0;
    uint32_t f = 0;
    uint32_t fDivisor = 1;
    bool pointEncountered = false;

    // max number of digits plus dot is 9
    for (uint32_t i = tokenStartIdx; i < minimal(tokenOneAfterEndIdx, tokenStartIdx + 9); ++i)
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
            return NAN;
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

bool parseUInt8(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx, uint8_t* pResult)
{
    if (!pResult || !pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx || tokenOneAfterEndIdx - tokenStartIdx > 3)
    {
        return false;
    }

    uint32_t r = 0;
    
    for (uint32_t i = tokenStartIdx; i < tokenOneAfterEndIdx; ++i)
    {
        if (!isdigit(pGpggaMessage->message[i]))
        {
            return false;
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

float parseLatLong(uint8_t numberOfDigitsInDegrees, Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
{
    if (!pGpggaMessage || tokenStartIdx >= tokenOneAfterEndIdx)
    {
        return NAN;
    }

    uint8_t degrees;

    if (!parseUInt8(pGpggaMessage, tokenStartIdx, minimal(tokenOneAfterEndIdx, tokenStartIdx + numberOfDigitsInDegrees), &degrees))
    {
        return NAN;
    }

    if (tokenStartIdx + numberOfDigitsInDegrees < tokenOneAfterEndIdx)
    {
        const float minutes = parseFloat(pGpggaMessage, tokenStartIdx + numberOfDigitsInDegrees, tokenOneAfterEndIdx);
        return (float) degrees + minutes / 60.0f;
    }
    else
    {
        return degrees;
    }
}

void parseGpsTime(GpsTime* pTime, Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
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

LATITUDE_HEMISPHERE parseLatitudeHemisphere(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
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

LONGITUDE_HEMISPHERE parseLongitudeHemisphere(Message* pGpggaMessage, uint32_t tokenStartIdx, uint32_t tokenOneAfterEndIdx)
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

void parseGpggaMessageIfValid(Message* pGpggaMessage, GpsData* pResult)
{
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
    parseGpsTime(&utcTime, pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // latitude
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const float latitudeDegrees = parseLatLong(2, pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // latitude hemisphere N/S
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const LATITUDE_HEMISPHERE latitudeHemisphereNS = parseLatitudeHemisphere(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    // longitude
    MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
    const float longitudeDegrees = parseLatLong(3, pGpggaMessage, tokenStartIdx, tokenOneAfterEndIdx);
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
        isnormal(latitudeDegrees) && 
        isnormal(longitudeDegrees))
    {
        pResult->isValid = true;
        pResult->utcTime = utcTime;
        pResult->latitudeDegrees = latitudeDegrees;
        pResult->latitudeHemisphere = latitudeHemisphereNS;
        pResult->longitudeDegrees = longitudeDegrees;
        pResult->longitudeHemisphere = longitudeHemisphereEW;
        pResult->fixType = fixType;
        pResult->altitudeMslMeters = altitudeMslMeters;
        pResult->numberOfSattelitesInUse = numberOfSattelitesInUse;
    }
}

void parseGpvtgMessageIfValid(Message* pGpvtgMessage, GpsData* pResult)
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
