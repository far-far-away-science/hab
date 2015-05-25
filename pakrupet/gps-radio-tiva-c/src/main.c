#define UART_BUFFERED
#define UART_RX_BUFFER_SIZE     1024
#define UART_TX_BUFFER_SIZE     128

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "../lib-c/uartstdio.h"

uint8_t calculateChecksum(uint8_t* pData, uint32_t bytesCount)
{
    if (bytesCount == 0 || !pData)
    {
        return 0;
    }
    uint8_t result = pData[0];
    for (uint8_t i = 1; i < bytesCount; ++i)
    {
        result ^= pData[i];
    }
    return result;
}

void ConfigureUart()
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(1, 9600, 16000000);
}

#define INPUT_BUFFER_LEN 256

static char pInputMessageBuffer[INPUT_BUFFER_LEN + 1];
static char pOutputMessageBuffer[128];

#pragma pack(push)
#pragma pack(1)
struct MessageReset
{
    uint8_t header1;
    uint8_t header2;
    uint16_t payloadSize;
    uint8_t messageId;
    uint8_t type;
    uint8_t checkSum;
    uint8_t footer1;
    uint8_t footer2;
};
struct MessageGetVersion
{
    uint8_t header1;
    uint8_t header2;
    uint16_t payloadSize;
    uint8_t messageId;
    uint8_t softwareType;
    uint8_t checkSum;
    uint8_t footer1;
    uint8_t footer2;
};
#pragma pack(pop)

int main()
{
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    ConfigureUart();

    struct MessageGetVersion* pMsg = (struct MessageGetVersion*) pOutputMessageBuffer;
    pMsg->header1 = '\xA0';
    pMsg->header2 = '\xA1';
    pMsg->payloadSize = 0x200;
    pMsg->messageId = 2;
    pMsg->softwareType = 1;
    pMsg->checkSum = calculateChecksum((uint8_t*) &pMsg->messageId, 2);
    pMsg->footer1 = '\x0D';
    pMsg->footer2 = '\x0A';
    UARTwrite(pOutputMessageBuffer, sizeof(struct MessageGetVersion));

    // library was fucking with the \x0D\x0A sequence and spoiling everything
    // need to reimplement my own stuff

    while (1)
    {
        memset(pInputMessageBuffer, 0, INPUT_BUFFER_LEN);
        int readSize = UARTgets(pInputMessageBuffer, INPUT_BUFFER_LEN);
        if (pInputMessageBuffer[0] == '\xA0' &&
            pInputMessageBuffer[1] == '\xA1')
        {
            if (pInputMessageBuffer[4] == '\x80')
            {
                // TODO received version info
            }
            if (pInputMessageBuffer[4] == '\x83')
            {
                // TODO success
            }
            if (pInputMessageBuffer[4] == '\x84')
            {
                // TODO failure
            }
        }
    }
}
