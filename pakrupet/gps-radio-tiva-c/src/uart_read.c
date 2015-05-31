#include "uart.h"
#include "uart_impl.h"

#include <string.h>

#include <driverlib/uart.h>
#include <driverlib/rom_map.h>

bool readMessage(uint8_t channel, struct Message* pResultBuffer)
{
    struct UartChannelData* const pChannelData = &uartChannelData[channel];

    if (!pChannelData->isFull && pChannelData->startIdx == pChannelData->endIdx)
    {
        return false;
    }
    memcpy(pResultBuffer, &pChannelData->buffer[pChannelData->startIdx], sizeof(struct Message));
    pChannelData->buffer[pChannelData->startIdx].size = 0;
    pChannelData->startIdx = advanceIndex(pChannelData->startIdx);
    return true;
}

void UartReadIntHandler(struct UartChannelData* pChannelData)
{
    static bool waitUntilNextCRLF = false;
    static bool previousCharWasCR = false;

    int32_t encodedChar;
    uint8_t decodedChar;

    while(MAP_UARTCharsAvail(pChannelData->base))
    {
        encodedChar = MAP_UARTCharGetNonBlocking(pChannelData->base);
        decodedChar = (uint8_t) (encodedChar & 0xFF);

        if (pChannelData->startIdx != pChannelData->endIdx && pChannelData->isFull)
        {
            pChannelData->isFull = false;
        }

        if (!pChannelData->isFull && !waitUntilNextCRLF)
        {
            const uint8_t charWriteIdx = pChannelData->buffer[pChannelData->endIdx].size;
            const bool thereIsSpaceForNewCharacter = charWriteIdx < UART_MESSAGE_MAX_LEN;

            if (thereIsSpaceForNewCharacter)
            {
                pChannelData->buffer[pChannelData->endIdx].message[charWriteIdx] = decodedChar;
                ++pChannelData->buffer[pChannelData->endIdx].size;
            }
            else
            {
                waitUntilNextCRLF = true; // message buffer overflow, skip until next CRLF sequence
            }

            if (!thereIsSpaceForNewCharacter || (previousCharWasCR && decodedChar == '\x0A'))
            {
                pChannelData->endIdx = advanceIndex(pChannelData->endIdx);
                if (pChannelData->endIdx == pChannelData->startIdx)
                {
                    pChannelData->isFull = true;
                }
                else
                {
                    pChannelData->buffer[pChannelData->endIdx].size = 0;
                }
            }
        }
        else
        {
            // put some code below to test full buffer scenarios
        }

        if (waitUntilNextCRLF && (previousCharWasCR && decodedChar == '\x0A'))
        {
            waitUntilNextCRLF = false;
        }

        if (decodedChar == '\x0D')
        {
            previousCharWasCR = true;
        }
        else
        {
            previousCharWasCR = false;
        }
    }
}
