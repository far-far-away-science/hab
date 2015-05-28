#include "tiva_c.h"
#include "signals.h"
#include "venus_gps.h"
#include "radio_board.h"
#include "telemetry_board.h"

#include <string.h>

bool findCommas(const uint8_t* pBuffer, uint8_t startIdx, uint8_t endIdx, uint8_t iterateUntilFoundCommasCount, uint8_t* pLastCommaIndex)
{
    if (!pLastCommaIndex)
    {
        return false;
    }
    uint8_t i = startIdx;
    uint8_t foundCommas = 0;
    for (; i <= endIdx; ++i)
    {
        if (pBuffer[i] == ',')
        {
            ++foundCommas;
            if (foundCommas == iterateUntilFoundCommasCount)
            {
                break;
            }
        }
    }
    *pLastCommaIndex = i;
    return foundCommas == iterateUntilFoundCommasCount;
}

bool createRadioBoardMessageFromGpggaMessage(uint8_t gpsSourceCode,
                                             const struct Message* pGpsGpggaMessage,
                                             struct Message* pRadioBoardGpsMessage)
{
    if (!pGpsGpggaMessage || !pRadioBoardGpsMessage || pGpsGpggaMessage->size < 60)
    {
        return false;
    }

    pRadioBoardGpsMessage->message[0] = gpsSourceCode;
    // time
    pRadioBoardGpsMessage->message[1] = ',';
    memcpy(&pRadioBoardGpsMessage->message[2], &pGpsGpggaMessage->message[7], 10);
    // latitude North
    pRadioBoardGpsMessage->message[12] = ',';
    memcpy(&pRadioBoardGpsMessage->message[13], &pGpsGpggaMessage->message[18], 9);
    // latitude West
    pRadioBoardGpsMessage->message[22] = ',';
    memcpy(&pRadioBoardGpsMessage->message[23], &pGpsGpggaMessage->message[30], 10);
    // altitude
    pRadioBoardGpsMessage->message[33] = ',';
    uint8_t altitudeStartIdx;
    if (!findCommas(pGpsGpggaMessage->message, 40, 52, 5, &altitudeStartIdx))
    {
        return false;
    }
    ++altitudeStartIdx; // advance to next character after comma
    uint8_t altitudeEndIdx;
    if (!findCommas(pGpsGpggaMessage->message, altitudeStartIdx, altitudeStartIdx + 7, 1, &altitudeEndIdx))
    {
        return false;
    }
    const uint8_t altitudeLen = altitudeEndIdx - altitudeStartIdx;
    memcpy(&pRadioBoardGpsMessage->message[34], &pGpsGpggaMessage->message[altitudeStartIdx], altitudeLen);
    pRadioBoardGpsMessage->message[34 + altitudeLen]     = '\x0D';
    pRadioBoardGpsMessage->message[34 + altitudeLen + 1] = '\x0A';
    pRadioBoardGpsMessage->size = 36 + altitudeLen;

    return true;
}

int main()
{
    initializeTivaC();
    initializeSignals();
    initializeVenusGps();

    signalSuccess();

    struct Message venusGpsMessage;
    struct Message radioBoardGpsMessage;

    const uint8_t venusGpsId = '1';

    while (true)
    {
        if (readVenusGpsMessage(&venusGpsMessage) && venusGpsMessage.size > 6)
        {
            if (memcmp(venusGpsMessage.message, "$GP", 3) == 0)
            {
                if (memcmp(venusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    writeMessageToTelemetryBoard(venusGpsId, &venusGpsMessage);
                    if (createRadioBoardMessageFromGpggaMessage(venusGpsId, &venusGpsMessage, &radioBoardGpsMessage))
                    {
                        writeMessageToRadioBoard(&radioBoardGpsMessage);
                    }
                }
                else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    writeMessageToTelemetryBoard(venusGpsId, &venusGpsMessage);
                }
            }
        }
    }
}
