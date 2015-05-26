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

void initializeVenusGps(void)
{
    // venus GPS runs on UART 1 (PortB)
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    if(!MAP_SysCtlPeripheralPresent(SYSCTL_PERIPH_UART1))
    {
        return;
    }
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    MAP_UARTConfigSetExpClk(UART1_BASE, 16000000, 9600, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8));
    MAP_UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    MAP_UARTIntDisable(UART1_BASE, 0xFFFFFFFF);
    MAP_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

    MAP_IntEnable(INT_UART1);

    MAP_UARTEnable(UART1_BASE);
}

/*
 * See startup_rvmdk.S file for declaration and mapping
 */
void Uart1IntHandler(void)
{
    uint32_t status = MAP_UARTIntStatus(UART1_BASE, true);
    MAP_UARTIntClear(UART1_BASE, status);

    if(status & (UART_INT_RX | UART_INT_RT))
    {
        // TODO read all received characters
    }
}
