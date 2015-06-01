#pragma once

#include <stdint.h>
#include <stdbool.h>

#define UART_NUMBER_OF_CHANNELS   4
#define UART_READ_BUFFER_MAX_LEN  3
#define UART_WRITE_BUFFER_MAX_LEN 3
#define UART_MESSAGE_MAX_LEN      128

struct Message
{
    uint8_t size;
    // to make sure there is no overflow check last 2 characters are 0x0D,0x0A.
    uint8_t message[UART_MESSAGE_MAX_LEN];
};

#define UART_0     0
#define UART_1     1
#define UART_2     2
#define UART_3     3
#define UART_4     4
#define UART_COUNT 5

#define UART_FLAGS_RECEIVE 0x01
#define UART_FLAGS_SEND    0x02

void initializeUart(void);
bool initializeUartChannel(uint8_t channel,
                           uint8_t uartPort,
                           uint32_t baudRate,
                           uint32_t cpuSpeedHz,
                           uint32_t flags);

// those functions should be used from main 'thread' only
// if you use them from other interrupts (higher priority than UART ones
// behaviour is undefined).
bool readMessage(uint8_t channel, struct Message* pResultBuffer);
bool writeMessage(uint8_t channel, const struct Message* pMessage);
