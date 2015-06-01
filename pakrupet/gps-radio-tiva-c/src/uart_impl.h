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
    struct Message buffer[UART_READ_BUFFER_MAX_LEN];
};

struct WriteBuffer
{
    bool isEmpty;
    uint8_t messageCharIdx;
    uint8_t startIdx;
    uint8_t endIdx;
    struct Message buffer[UART_WRITE_BUFFER_MAX_LEN];
};

struct UartChannelData
{
    uint32_t base;
    uint32_t interruptId;
    struct ReadBuffer readBuffer;
    struct WriteBuffer writeBuffer;
};

uint8_t advanceIndex(uint8_t currentValue, uint8_t maxLen);

void UartReadIntHandler(struct UartChannelData* pChannelData);
void UartWriteIntHandler(struct UartChannelData* pChannelData);

extern struct UartChannelData uartChannelData[UART_NUMBER_OF_CHANNELS];
