#include "uart.h"
#include "uart_impl.h"

#include <string.h>

#include <driverlib/uart.h>
#include <driverlib/rom_map.h>

bool readMessage(uint8_t channel, struct Message* pResultBuffer)
{
    struct UartChannelData* const pChannelData = &uartChannelData[channel];

    // if buffer is empty and interrupt kicks in after we check '!pChannelData->readBuffer.isFull' we are still fine
    // as buffer can only get fuller by interrupt
    if (!pChannelData->readBuffer.isFull && pChannelData->readBuffer.endIdx == pChannelData->readBuffer.startIdx)
    {
        return false;
    }

    // start index can only be changed by 'thread' other than UART interrupt handler
    // so we fine here as well
    memcpy(pResultBuffer, &pChannelData->readBuffer.buffer[pChannelData->readBuffer.startIdx], sizeof(struct Message));
    pChannelData->readBuffer.buffer[pChannelData->readBuffer.startIdx].size = 0;
    pChannelData->readBuffer.startIdx = advanceIndex(pChannelData->readBuffer.startIdx, UART_READ_BUFFER_MAX_LEN);

    return true;
}

void UartReadIntHandler(struct UartChannelData* pChannelData)
{
    int32_t encodedChar;
    uint8_t decodedChar;

    while(MAP_UARTCharsAvail(pChannelData->base))
    {
        encodedChar = MAP_UARTCharGetNonBlocking(pChannelData->base);
        decodedChar = (uint8_t) (encodedChar & 0xFF);

        if (pChannelData->readBuffer.isFull && pChannelData->readBuffer.startIdx != pChannelData->readBuffer.endIdx)
        {
            // other 'thread' advanced start index (doesn't matter much if we miss this event and think that buffer is
            // full for the next iteration)
            pChannelData->readBuffer.isFull = false;
        }

        if (!pChannelData->readBuffer.isFull && !pChannelData->readBuffer.waitUntilNextCRLF)
        {
            const uint8_t charWriteIdx = pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size;
            const bool thereIsSpaceForNewCharacter = charWriteIdx < UART_MESSAGE_MAX_LEN;

            if (thereIsSpaceForNewCharacter)
            {
                pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].message[charWriteIdx] = decodedChar;
                ++pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size;
            }
            else
            {
                pChannelData->readBuffer.waitUntilNextCRLF = true; // message buffer overflow, skip until next CRLF sequence
            }

            if (!thereIsSpaceForNewCharacter || (pChannelData->readBuffer.previousCharWasCR && decodedChar == '\x0A'))
            {
                // start index can only get away and cannot get past end index so we are fine here
                pChannelData->readBuffer.endIdx = advanceIndex(pChannelData->readBuffer.endIdx, UART_READ_BUFFER_MAX_LEN);
                // if start index moves on we are golden and can continue writing
                if (pChannelData->readBuffer.endIdx == pChannelData->readBuffer.startIdx)
                {
                    pChannelData->readBuffer.isFull = true;
                }
                else
                {
                    pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size = 0;
                }
            }
        }
        else
        {
            // put some code below to test full buffer scenarios
        }

        if (pChannelData->readBuffer.waitUntilNextCRLF && (pChannelData->readBuffer.previousCharWasCR && decodedChar == '\x0A'))
        {
            pChannelData->readBuffer.waitUntilNextCRLF = false;
        }

        if (decodedChar == '\x0D')
        {
            pChannelData->readBuffer.previousCharWasCR = true;
        }
        else
        {
            pChannelData->readBuffer.previousCharWasCR = false;
        }
    }
}
