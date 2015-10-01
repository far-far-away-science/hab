#include "nmea_messages_impl.h"

#include <math.h>
#include <ctype.h>
#include <stdlib.h>

int32_t floatAngularCoordinateToInt32Degrees(AngularCoordinate lat)
{
    if (lat.isValid)
    {
        return (int32_t) (1000000.0f * (lat.degrees + lat.minutes / 60.0f));
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
