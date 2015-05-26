#include "venus_gps.h"
#include "venus_gps_internal.h"

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>

#define VENUS_GPS_UART_INT INT_UART1
#define VENUS_GPS_UART_BASE UART1_BASE
#define VENUS_GPS_UART_SYS_CTL SYSCTL_PERIPH_UART1

#define VENUS_GPS_BUFFER_SIZE 3

struct VenusGpsStatics
{
    bool messageIsBeingRead;

    int8_t messageBeingWrittenIdx;
    int8_t messageAvailableForReadingIdx;

    struct VenusGpsMessage buffer[VENUS_GPS_BUFFER_SIZE];
} gVenusGps;

void initializeVenusGps(void)
{
    // venus GPS runs on UART 1 (PortB)
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    if(!MAP_SysCtlPeripheralPresent(VENUS_GPS_UART_SYS_CTL))
    {
        return;
    }
    MAP_SysCtlPeripheralEnable(VENUS_GPS_UART_SYS_CTL);

    UARTClockSourceSet(VENUS_GPS_UART_BASE, UART_CLOCK_PIOSC);
    MAP_UARTConfigSetExpClk(VENUS_GPS_UART_BASE, 16000000, 9600, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8));
    MAP_UARTFIFOLevelSet(VENUS_GPS_UART_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    MAP_UARTIntDisable(VENUS_GPS_UART_BASE, 0xFFFFFFFF);
    MAP_UARTIntEnable(VENUS_GPS_UART_BASE, UART_INT_RX | UART_INT_RT);

    MAP_IntEnable(VENUS_GPS_UART_INT);

    MAP_UARTEnable(VENUS_GPS_UART_BASE);
}

int8_t getNextWriteBufferIdx()
{
    return 0; // TODO advance to next available write spot
}

/*
 * See startup_rvmdk.S file for declaration and mapping
 */
void Uart1IntHandler(void)
{
    static bool previousCharWasCR = false;

    uint32_t status = MAP_UARTIntStatus(VENUS_GPS_UART_BASE, true);
    MAP_UARTIntClear(VENUS_GPS_UART_BASE, status);

    if(status & (UART_INT_RX | UART_INT_RT))
    {
        int32_t encodedChar;
        uint8_t decodedChar;

        while(MAP_UARTCharsAvail(VENUS_GPS_UART_BASE))
        {
            encodedChar = MAP_UARTCharGetNonBlocking(VENUS_GPS_UART_BASE);
            decodedChar = (uint8_t) (encodedChar & 0xFF);

            if (gVenusGps.messageBeingWrittenIdx == -1)
            {
                gVenusGps.messageBeingWrittenIdx = getNextWriteBufferIdx();
                gVenusGps.buffer[gVenusGps.messageBeingWrittenIdx].size = 0;
            }

            if (gVenusGps.messageBeingWrittenIdx < VENUS_GPS_BUFFER_SIZE)
            {
                const uint8_t writeIdx = gVenusGps.buffer[gVenusGps.messageBeingWrittenIdx].size;
                const bool thereIsSpaceForNewCharacter = writeIdx < VENUS_GPS_MESSAGE_MAX_LEN;

                if (thereIsSpaceForNewCharacter)
                {
                    gVenusGps.buffer[gVenusGps.messageBeingWrittenIdx].message[writeIdx] = decodedChar;
                    ++gVenusGps.buffer[gVenusGps.messageBeingWrittenIdx].size;
                }

                if (!thereIsSpaceForNewCharacter || (previousCharWasCR && decodedChar == '\x0A'))
                {
                    gVenusGps.messageBeingWrittenIdx = -1;
                }
            }

            if (decodedChar == '\x0D')
            {
                previousCharWasCR = true;
            }
            else
            {
                previousCharWasCR = false;
            }
        }
    }
}
