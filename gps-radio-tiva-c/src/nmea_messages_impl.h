#pragma once

#include "nmea_messages.h"

#define MOVE_TO_NEXT_RETURN_IF_END_OF_STRING_REACHED(pMessage, tokenStartIdx, tokenOneAfterEndIdx) \
    (tokenStartIdx) = (tokenOneAfterEndIdx) + 1; \
    (tokenOneAfterEndIdx) = findDivider((pMessage), (tokenStartIdx)); \
    if ((tokenOneAfterEndIdx) >= (pMessage)->size) { return; };

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
