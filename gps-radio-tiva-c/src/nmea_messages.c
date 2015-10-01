#include "nmea_messages_impl.h"

#include <math.h>

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
