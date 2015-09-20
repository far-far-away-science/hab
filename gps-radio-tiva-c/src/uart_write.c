#include "uart.h"
#include "uart_impl.h"

#include <string.h>

uint16_t advanceUint16Index(uint16_t currentValue, uint16_t maxLen)
{
    ++currentValue;
    if (currentValue >= maxLen)
    {
        currentValue = 0;
    }
    return currentValue;
}

uint16_t getBufferCapacity(const WriteBuffer* pWriteBuffer, uint16_t maxLen)
{
    const bool isEmpty = pWriteBuffer->isEmpty;
    const uint16_t start = pWriteBuffer->startIdx;
    const uint16_t end = pWriteBuffer->endIdx;
    
    if (start == end)
    {
        if (isEmpty)
        {
            return maxLen;
        }
        else
        {
            return 0;
        }
    }
    else if (end > start)
    {
        return maxLen - (end - start);
    }
    else
    {
        return end - start;
    }
}

void uartTransmit(UartChannelData* pChannelData)
{
    UARTTxInterruptDisable(pChannelData);
    
    while(UARTSpaceAvailable(pChannelData) && 
          (pChannelData->writeBuffer.endIdx != pChannelData->writeBuffer.startIdx || !pChannelData->writeBuffer.isEmpty))
    {
        UARTPutCharNonBlocking(pChannelData, pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.startIdx]);
        pChannelData->writeBuffer.startIdx = advanceUint16Index(pChannelData->writeBuffer.startIdx, UART_WRITE_BUFFER_MAX_CHARS_LEN);
        if (pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) 
        {
            pChannelData->writeBuffer.isEmpty = true;
        }
    }

    UARTTxInterruptEnable(pChannelData);
}

bool writeString(uint8_t channel, char* szData)
{
    uint8_t size = strlen(szData);
    return writeMessageBuffer(channel, (uint8_t*) szData, size);
}

bool write(uint8_t channel, uint8_t character)
{
    UartChannelData* const pChannelData = &uartChannelData[channel];

    // interrupt can only make buffer emptier so we are fine no matter where interrupt kicks in
    if ((!pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) ||
        getBufferCapacity(&pChannelData->writeBuffer, UART_WRITE_BUFFER_MAX_CHARS_LEN) < 1)
    {
        return false;
    }

    pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.endIdx] = character;
    pChannelData->writeBuffer.endIdx = advanceUint16Index(pChannelData->writeBuffer.endIdx, UART_WRITE_BUFFER_MAX_CHARS_LEN);
        
    uartTransmit(pChannelData);
        
    return true;
}

bool writeMessage(uint8_t channel, const Message* pMessage)
{
    return writeMessageBuffer(channel, pMessage->message, pMessage->size);
}

bool writeMessageBuffer(uint8_t channel, const uint8_t* pBuffer, uint8_t size)
{
    UartChannelData* const pChannelData = &uartChannelData[channel];

    // interrupt can only make buffer emptier so we are fine no matter where interrupt kicks in
    if ((!pChannelData->writeBuffer.isEmpty && pChannelData->writeBuffer.startIdx == pChannelData->writeBuffer.endIdx) ||
        getBufferCapacity(&pChannelData->writeBuffer, UART_WRITE_BUFFER_MAX_CHARS_LEN) < size)
    {
        return false;
    }

    for (uint8_t i = 0; i < size; ++i)
    {
        pChannelData->writeBuffer.buffer[pChannelData->writeBuffer.endIdx] = pBuffer[i];
        pChannelData->writeBuffer.endIdx = advanceUint16Index(pChannelData->writeBuffer.endIdx, UART_WRITE_BUFFER_MAX_CHARS_LEN);
    }
        
    uartTransmit(pChannelData);
        
    return true;
}

void uartWriteIntHandler(UartChannelData* pChannelData)
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
        UARTTxInterruptDisable(pChannelData);
    }
}
