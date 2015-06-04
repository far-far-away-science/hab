#include "uart.h"
#include "uart_impl.h"

#include <string.h>

#include <driverlib/uart.h>
#include <driverlib/rom_map.h>
#include <driverlib/interrupt.h>

void UartTransmit(struct UartChannelData* pChannelData)
{
    // this function can be called from main 'thread' and from the interrupt
    // need to make sure that we aren't interrupted while executing in the
    // main 'thread' or we will get race condition and unexpected behaviour
    
    MAP_UARTIntDisable(pChannelData->base, UART_INT_TX);

    while(MAP_UARTSpaceAvail(pChannelData->base))
    {
        if (pChannelData->writeBuffer.messageCharIdx < pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.startIdx].size)
        {
            MAP_UARTCharPutNonBlocking(
                pChannelData->base, 
                pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.startIdx].message[pChannelData->writeBuffer.messageCharIdx]
            );
            ++pChannelData->writeBuffer.messageCharIdx;
        }
        else
        {
            pChannelData->writeBuffer.messageCharIdx = 0;
            pChannelData->writeBuffer.startIdx = advanceIndex(pChannelData->writeBuffer.startIdx, UART_WRITE_BUFFER_MAX_LEN);

            if (pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) 
            {
                pChannelData->writeBuffer.isEmpty = true;
                break;
            }
        }
    }

    MAP_UARTIntEnable(pChannelData->base, UART_INT_TX);
}

bool writeMessage(uint8_t channel, const struct Message* pMessage)
{
    struct UartChannelData* const pChannelData = &uartChannelData[channel];

    // interrupt can only make buffer emptier so we are fine no matter where interrupt kicks in
    if (!pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx)
    {
        return false;
    }

    memcpy(&pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.endIdx], pMessage, sizeof(struct Message));
    pChannelData->writeBuffer.endIdx = advanceIndex(pChannelData->writeBuffer.endIdx, UART_WRITE_BUFFER_MAX_LEN);
    MAP_UARTIntEnable(pChannelData->base, UART_INT_TX);
    UartTransmit(pChannelData);
    return true;
}

void UartWriteIntHandler(struct UartChannelData* pChannelData)
{
    bool disableInterrupt = false;

    if (pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx)
    {
        disableInterrupt = true;
    }
    else
    {
        UartTransmit(pChannelData);
        if (pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx)
        {
            disableInterrupt = true;
        }
    }

    if (disableInterrupt)
    {
        // user shouldn't call writeMessage from interrupt handlers
        // so nobody can write message without enabling this interrupt
        MAP_UARTIntDisable(pChannelData->base, UART_INT_TX);
    }
}
