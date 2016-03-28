#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include <DeviceSpecific.h>

VOID WriteRegisterUShort(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset, _In_ USHORT value);
USHORT ReadRegisterUShort(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset);

#define REGISTER_FR 0x18 // flag register
    #define REGISTER_FR_BUSY (1 << 3) // uart is busy

FORCEINLINE USHORT REGISTER_FR_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return ReadRegisterUShort(baseAddress, REGISTER_FR);
}

/*
* do not edit this register when UART is enabled or
* UART is completing transmission or reception when UART disabled flag was set
*/
#define REGISTER_LCRH 0x2C // line control register
    #define REGISTER_LCRH_FEN (1 << 4) // FIFO enable bit

FORCEINLINE USHORT REGISTER_LCRH_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return ReadRegisterUShort(baseAddress, REGISTER_LCRH);
}

FORCEINLINE VOID REGISTER_LCRH_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT value)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    WriteRegisterUShort(baseAddress, REGISTER_LCRH, value);
}

/*
* do not edit this register when UART is enabled or
* UART is completing transmission or reception when UART disabled flag was set
*/
#define REGISTER_CR 0x30 // modem control register
    #define REGISTER_CR_UARTEN (1 <<  0) // enable or disable UART
    #define REGISTER_CR_RTS    (1 << 11)

FORCEINLINE USHORT REGISTER_CR_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return ReadRegisterUShort(baseAddress, REGISTER_CR);
}

FORCEINLINE VOID REGISTER_CR_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT value)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    WriteRegisterUShort(baseAddress, REGISTER_CR, value);
}

VOID EnableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
VOID DisableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
