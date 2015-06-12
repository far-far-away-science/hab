#pragma once

#include <stdint.h>
#include <stdbool.h>

struct ReadBuffer
{
    bool isFull;
    bool waitUntilNextCRLF;
    bool previousCharWasCR;
    uint8_t startIdx;
    uint8_t endIdx;
    struct Message buffer[UART_READ_BUFFER_MAX_MESSAGES_LEN];
};

struct WriteBuffer
{
    bool isEmpty;
    uint16_t startIdx;
    uint16_t endIdx;
    uint8_t buffer[UART_WRITE_BUFFER_MAX_CHARS_LEN];
};

struct UartChannelData
{
    uint32_t base;
    uint32_t interruptId;
    struct ReadBuffer readBuffer;
    struct WriteBuffer writeBuffer;
};

void uartReadIntHandler(struct UartChannelData* pChannelData);
void uartWriteIntHandler(struct UartChannelData* pChannelData);

extern struct UartChannelData uartChannelData[UART_NUMBER_OF_CHANNELS];
