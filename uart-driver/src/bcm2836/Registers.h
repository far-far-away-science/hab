#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\CommonDefinitions.h"

#include "..\..\Trace.h"

#define AUX_MU_IIR_REGISTER 0x48 // interrupt identity register
#define AUX_MU_IIR_REGISTER_W_RCVR_RESET (1 << 1) // write 1 to reset receive FIFO
#define AUX_MU_IIR_REGISTER_W_TXMT_RESET (1 << 2) // write 1 to reset transmit FIFO

#define AUX_MU_LSR_REGISTER 0x54 // line status register
#define AUX_MU_MSR_REGISTER 0x58 // modem status register

FORCEINLINE VOID WRITE_FIFO_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR controlValue)
{
    REGBASE baseAddress;
    baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IIR_REGISTER, controlValue);
}

FORCEINLINE UCHAR READ_LINE_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    REGBASE baseAddress;
    baseAddress = pDeviceExtension->ControllerAddress;
    const UCHAR result = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LSR_REGISTER);
    return result;
}

FORCEINLINE UCHAR READ_MODEM_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    REGBASE baseAddress;
    baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_MSR_REGISTER);
}
