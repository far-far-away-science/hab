#pragma once

#include <stdint.h>
#include <stdbool.h>

struct UartChannelData
{
    uint32_t base;
    uint32_t interruptId;
    bool isFull;
    uint8_t startIdx;
    uint8_t endIdx;
    struct Message buffer[UART_BUFFER_MAX_LEN];
};

uint8_t advanceIndex(uint8_t currentValue);

void UartReadIntHandler(struct UartChannelData* pChannelData);
void UartWriteIntHandler(struct UartChannelData* pChannelData);

extern struct UartChannelData uartChannelData[UART_NUMBER_OF_CHANNELS];
