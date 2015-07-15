#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include <DeviceDefinitions.h>

/*
* do not edit this register when UART is enabled or
* UART is completing transmission or reception when UART disabled flag was set
*/
#define REGISTER_LCRH 0x2C // line control register
    #define REGISTER_LCRH_FEN (1 << 4) // FIFO enable bit

FORCEINLINE VOID REGISTER_LCRH_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR value)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, REGISTER_LCRH, value);
}

/*
* do not edit this register when UART is enabled or
* UART is completing transmission or reception when UART disabled flag was set
*/
#define REGISTER_CR 0x30 // modem control register
    #define REGISTER_CR_UARTEN (1 << 0) // enable or disable UART

FORCEINLINE UCHAR REGISTER_CR_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartReadDeviceUChar(baseAddress, REGISTER_CR + 0);
    pDeviceExtension->UartReadDeviceUChar(baseAddress, REGISTER_CR + 1);
    pDeviceExtension->UartReadDeviceUChar(baseAddress, REGISTER_CR + 2);
    pDeviceExtension->UartReadDeviceUChar(baseAddress, REGISTER_CR + 3);
    // pDeviceExtension->UartReadDeviceUChar(baseAddress, REGISTER_CR);
    return 0;
}

FORCEINLINE VOID REGISTER_CR_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR value)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, REGISTER_CR, value);
}

VOID EnableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
VOID DisableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
