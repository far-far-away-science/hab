#include "uart.h"
#include "uart_impl.h"

#include <string.h>

uint8_t advanceUint8Index(uint8_t currentValue, uint8_t maxLen)
{
    ++currentValue;
    if (currentValue >= maxLen)
    {
        currentValue = 0;
    }
    return currentValue;
}

bool readMessage(uint8_t channel, Message* pResultBuffer)
{
    if (!pResultBuffer)
    {
        return false;
    }
    
    UartChannelData* const pChannelData = &uartChannelData[channel];

    // if buffer is empty and interrupt kicks in after we check '!pChannelData->readBuffer.isFull' we are still fine
    // as buffer can only get fuller by interrupt
    if (!pChannelData->readBuffer.isFull && pChannelData->readBuffer.endIdx == pChannelData->readBuffer.startIdx)
    {
        return false;
    }

    // start index can only be changed by 'thread' other than UART interrupt handler
    // so we fine here as well
    memcpy(pResultBuffer, &pChannelData->readBuffer.buffer[pChannelData->readBuffer.startIdx], sizeof(Message));
    pChannelData->readBuffer.buffer[pChannelData->readBuffer.startIdx].size = 0;
    pChannelData->readBuffer.startIdx = advanceUint8Index(pChannelData->readBuffer.startIdx, UART_READ_BUFFER_MAX_MESSAGES_LEN);

    return pResultBuffer->size > 0;
}

void uartReadIntHandler(UartChannelData* pChannelData)
{
    int32_t encodedChar;
    uint8_t decodedChar;

    while(UARTCharactersAvailable(pChannelData))
    {
        encodedChar = UARTGetCharNonBlocking(pChannelData);
        decodedChar = (uint8_t) (encodedChar & 0xFF);

        if (pChannelData->readBuffer.isFull && pChannelData->readBuffer.startIdx != pChannelData->readBuffer.endIdx)
        {
            // other 'thread' advanced start index (doesn't matter much if we miss this event and think that buffer is
            // full for the next iteration)
            pChannelData->readBuffer.isFull = false;
        }

        if (!pChannelData->readBuffer.isFull && !pChannelData->readBuffer.waitUntilNextMessage)
        {
            const uint8_t charWriteIdx = pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size;
            const bool thereIsSpaceForNewCharacter = charWriteIdx < UART_MESSAGE_MAX_LEN;

            bool moveToNewMessage = false;
            bool placeCurrentCharToNewMessage = false;
            
            if (!thereIsSpaceForNewCharacter)
            {
                moveToNewMessage = true;
                pChannelData->readBuffer.waitUntilNextMessage = true; // message buffer overflow, skip until next message
            }
            else if (decodedChar == '$' && charWriteIdx != 0)
            {
                moveToNewMessage = true;
                placeCurrentCharToNewMessage = true;
                pChannelData->readBuffer.waitUntilNextMessage = false; // we already at the beginning of next message
            }                
            else
            {
                pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].message[charWriteIdx] = decodedChar;
                ++pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size;
            }
            
            if (moveToNewMessage)
            {
                // start index can only get away and cannot get past end index so we are fine here
                pChannelData->readBuffer.endIdx = advanceUint8Index(pChannelData->readBuffer.endIdx, UART_READ_BUFFER_MAX_MESSAGES_LEN);
                // if start index moves on we are golden and can continue writing
                if (pChannelData->readBuffer.endIdx == pChannelData->readBuffer.startIdx)
                {
                    pChannelData->readBuffer.isFull = true;
                }
                else
                {
                    if (placeCurrentCharToNewMessage)
                    {
                        pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].message[0] = decodedChar;
                        pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size = 1;
                    }
                    else
                    {
                        pChannelData->readBuffer.buffer[pChannelData->readBuffer.endIdx].size = 0;
                    }
                }
            }
        }
        else
        {
            // put some code below to test full buffer scenarios
        }

        if (pChannelData->readBuffer.waitUntilNextMessage && (pChannelData->readBuffer.previousCharWasCR && decodedChar == '\x0A'))
        {
            pChannelData->readBuffer.waitUntilNextMessage = false;
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
