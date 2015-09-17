#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef DUMP_DATA_TO_UART0
    #define UART_NUMBER_OF_CHANNELS 3
#else
    #define UART_NUMBER_OF_CHANNELS 2
#endif

#define UART_READ_BUFFER_MAX_MESSAGES_LEN 3
#define UART_WRITE_BUFFER_MAX_CHARS_LEN   512
#define UART_MESSAGE_MAX_LEN              128

typedef struct Message_t
{
    uint8_t size;
    // to make sure there is no overflow check last 2 characters are 0x0D,0x0A.
    uint8_t message[UART_MESSAGE_MAX_LEN];
} Message;

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
bool readMessage(uint8_t channel, Message* pResultBuffer);
bool write(uint8_t channel, uint8_t character);
bool writeMessage(uint8_t channel, const Message* pMessage);
