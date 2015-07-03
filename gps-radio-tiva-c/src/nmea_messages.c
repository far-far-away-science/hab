/*
 * nmea_messages.c - Source code derived from the GPS parser I (stcarlso) wrote for the
 * open-source Cornell Cup 2014 project (https://github.com/snowpuppy/augreality)
 */

#include "nmea_messages.h"

/**
 * Tokenizes a string, like strtok(), but re-entrantly.
 *
 * @param string a pointer to the start of the string
 * @param split the character used to split
 * @param token the buffer where the token will be stored
 * @param length the length of the buffer
 * @return the length of the string parsed
 */
static uint32_t tokenize(char **string, char split, char *token, uint32_t length) {
    char *str = *string; uint32_t count = 0;
    // Iterate until token hit or end of string
    while (*str && *str != split && count < length) {
        *token++ = *str++;
        count++;
    }
    // Skip the token and null terminate
    if (*str) str++;
    *token = '\0';
    *string = str;
    return count;
}

/**
 * Parses a number from the null-terminated buffer. Decimal points are ignored.
 *
 * @param buffer the buffer of numeric data
 * @return the number parsed, or 0 if no number could be parsed
 */
static int32_t parseNumber(char *buffer) {
    int32_t value = 0; char c;
    while (*buffer) {
        c = *buffer++;
        if (c >= '0' && c <= '9')
            value = (value * 10) + (int32_t)(c - '0');
    }
    return value;
}

/**
 * Parses a number from the null-terminated buffer into GPS decimal degrees. Performs
 * conversion from degrees and minutes into degrees * 1E6.
 *
 * @param buffer the buffer of numeric data
 * @return the number parsed, or 0 if no number could be parsed
 */
static int32_t parseLatLon(char *buffer) {
    // It's in format dd mm.mmmm so convert properly to dd.dddddd
    // Still unsigned here as we haven't parsed the "-" or "N/S/E/W"
    int32_t value = parseNumber(buffer);
    return (value / 1000000) * 1000000 + (value % 1000000) * 5 / 3;
}

void parseGpggaMessageIfValid(const Message* pGpggaMessage, GpsData* pResult)
{
    int32_t lat, lon;
    char gpt[32];
    // Start after the "$GPGGA," and ensure null termination is in place
    char *str = (char *)&(pGpggaMessage->message);
    str[pGpggaMessage->size] = '\0';
    str += 7;
    pResult->isValid = false;
    // Time (x10)
    tokenize(&str, ',', gpt, sizeof(gpt));
    pResult->utcTime = parseNumber(gpt) / 10U;
    // Latitude
    tokenize(&str, ',', gpt, sizeof(gpt));
    lat = parseLatLon(gpt);
    // N/S
    tokenize(&str, ',', gpt, sizeof(gpt));
    if (gpt[0] == 'S') lat = -lat;
    // Longitude
    tokenize(&str, ',', gpt, sizeof(gpt));
    lon = parseLatLon(gpt);
    // E/W
    tokenize(&str, ',', gpt, sizeof(gpt));
    if (gpt[0] == 'W') lon = -lon;
    // Fix?
    tokenize(&str, ',', gpt, sizeof(gpt));
    if (gpt[0] == '1')
    {
        // GPS satellites in view
        tokenize(&str, ',', gpt, sizeof(gpt));
        pResult->numberOfSatellites = parseNumber(gpt);
        // HDOP
        tokenize(&str, ',', gpt, sizeof(gpt));
        // Altitude M (always has 1 decimal place)
        tokenize(&str, ',', gpt, sizeof(gpt));
        pResult->altitudeMeters = parseNumber(gpt) * 10U;
        // Valid result, and have fix
        pResult->isValid = true;
        pResult->gpsQualityIndicator = 1U;
        pResult->latitude = lat;
        pResult->longitude = lon;
        // True Course and Speed cannot be determined from GPGGA
    }
    else
    {
        // No GPS fix
        pResult->gpsQualityIndicator = 0U;
        pResult->numberOfSatellites = 0U;
    }
}

void parseGpvtgMessageIfValid(const Message* pGpvtgMessage, GpsData* pResult)
{
    char gpt[32];
    // Start after the "$GPVTG," and ensure null termination is in place
    char *str = (char *)&(pGpvtgMessage->message);
    str[pGpvtgMessage->size] = '\0';
    str += 7;
    // True track made good in degrees * 10
    tokenize(&str, ',', gpt, sizeof(gpt));
    pResult->trueCourseDegrees = parseNumber(gpt);
    // "T"
    tokenize(&str, ',', gpt, sizeof(gpt));
    // Magnetic track made good in degrees * 10
    tokenize(&str, ',', gpt, sizeof(gpt));
    // "M"
    tokenize(&str, ',', gpt, sizeof(gpt));
    // Ground speed knots * 10
    tokenize(&str, ',', gpt, sizeof(gpt));
    // "N"
    tokenize(&str, ',', gpt, sizeof(gpt));
    // Ground speed km/h * 10
    tokenize(&str, ',', gpt, sizeof(gpt));
    pResult->speedKmh = parseNumber(gpt);
}
