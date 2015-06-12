#include "uart.h"
#include "uart_impl.h"

#include <string.h>

#include <driverlib/uart.h>
#include <driverlib/rom_map.h>
#include <driverlib/interrupt.h>

uint16_t advanceUint16Index(uint16_t currentValue, uint16_t maxLen)
{
    ++currentValue;
    if (currentValue >= maxLen)
    {
        currentValue = 0;
    }
    return currentValue;
}

uint16_t getBufferCapacity(const struct WriteBuffer* pWriteBuffer, uint16_t maxLen)
{
		const bool isEmpty = pWriteBuffer->isEmpty;
		const uint16_t start = pWriteBuffer->startIdx;
		const uint16_t end = pWriteBuffer->endIdx;
	
	  if (isEmpty && start == end)
		{
	      return maxLen;
		}
		else if (end >= start)
		{
				return end - start;
		}
		else
		{
				return maxLen - start + end;
		}
}

void uartTransmit(struct UartChannelData* pChannelData)
{
    while(MAP_UARTSpaceAvail(pChannelData->base) && !pChannelData->writeBuffer.isEmpty)
    {
					MAP_UARTCharPutNonBlocking(pChannelData->base, pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.startIdx]);
					pChannelData->writeBuffer.startIdx = advanceUint16Index(pChannelData->writeBuffer.startIdx, UART_WRITE_BUFFER_MAX_CHARS_LEN);
					if (pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) 
					{
							pChannelData->writeBuffer.isEmpty = true;
					}
    }
}

bool writeMessage(uint8_t channel, const struct Message* pMessage)
{
    struct UartChannelData* const pChannelData = &uartChannelData[channel];

    // interrupt can only make buffer emptier so we are fine no matter where interrupt kicks in
    if ((!pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) ||
		    getBufferCapacity(&pChannelData->writeBuffer, UART_WRITE_BUFFER_MAX_CHARS_LEN) < pMessage->size)
    {
        return false;
    }

		for (uint8_t i = 0; i < pMessage->size; ++i)
		{
				pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.endIdx] = pMessage->message[i];
				pChannelData->writeBuffer.endIdx = advanceUint16Index(pChannelData->writeBuffer.endIdx, UART_WRITE_BUFFER_MAX_CHARS_LEN);
		}
		
    uartTransmit(pChannelData);
    MAP_UARTIntEnable(pChannelData->base, UART_INT_TX);
		
    return true;
}

void uartWriteIntHandler(struct UartChannelData* pChannelData)
{
    bool disableInterrupt = false;

    if (pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx)
    {
        disableInterrupt = true;
    }
    else
    {
        uartTransmit(pChannelData);
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
