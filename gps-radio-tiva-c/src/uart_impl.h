#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct ReadBuffer_t
{
    bool isFull;
    bool waitUntilNextCRLF;
    bool previousCharWasCR;
    uint8_t startIdx;
    uint8_t endIdx;
    Message buffer[UART_READ_BUFFER_MAX_MESSAGES_LEN];
} ReadBuffer;

typedef struct WriteBuffer_t
{
    bool isEmpty;
    uint16_t startIdx;
    uint16_t endIdx;
    uint8_t buffer[UART_WRITE_BUFFER_MAX_CHARS_LEN];
} WriteBuffer;

typedef struct UartChannelData_t
{
    uint32_t base;
    uint32_t interruptId;
    ReadBuffer readBuffer;
    WriteBuffer writeBuffer;
} UartChannelData;

void uartReadIntHandler(UartChannelData* pChannelData);
void uartWriteIntHandler(UartChannelData* pChannelData);

extern UartChannelData uartChannelData[UART_NUMBER_OF_CHANNELS];
