#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\CommonDefinitions.h"

#include "..\..\Trace.h"

#define AUX_ENABLES 0x04
    #define AUX_ENABLES_ENABLE_UART (1 << 0)

#define AUX_MU_IO_REGISTER 0x40 // I/O register

#define AUX_MU_IER_REGISTER 0x44 // interrupt enable register
    #define AUX_MU_IER_REGISTER_W_INT_ENABLE_RECEIVE            (1 << 0)
    #define AUX_MU_IER_REGISTER_W_INT_ENABLE_TRANSMIT           (1 << 1)
    #define AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE (1 << 2)

#define AUX_MU_IIR_REGISTER 0x48 // interrupt identity register
    #define AUX_MU_IIR_REGISTER_W_RESET_RECEIVE_FIFO  (1 << 1)
    #define AUX_MU_IIR_REGISTER_W_RESET_TRANSMIT_FIFO (1 << 2)

#define AUX_MU_LCR_REGISTER 0x4C // line control register
    #define AUX_MU_LCR_REGISTER_W_DLAB (1 << 7) // this is a flag which chooses what some flags in other registers mean

#define AUX_MU_MCR_REGISTER 0x50 // modem control register
    #define AUX_MU_MCR_REGISTER_W_REQUEST_TO_SENT (1 << 1) // (if 1 then RTS line is low)

#define AUX_MU_LSR_REGISTER 0x54 // line status register
    #define AUX_MU_LSR_REGISTER_R_DATA_READY        (1 << 0)
    #define AUX_MU_LSR_REGISTER_R_OVERRUN_ERROR     (1 << 1)
    #define AUX_MU_LSR_REGISTER_R_TRANSMITTER_EMPTY (1 << 5)
    #define AUX_MU_LSR_REGISTER_R_TRANSMITTER_IDLE  (1 << 6)

#define AUX_MU_MSR_REGISTER 0x58 // modem status register

// only works if DLAB is 1
#define AUX_MU_BAUD_RATE_LSB_REGISTER AUX_MU_IO_REGISTER // least significant byte
// only works if DLAB is 1
#define AUX_MU_BAUD_RATE_MSB_REGISTER AUX_MU_IER_REGISTER // most significan byte

FORCEINLINE VOID WRITE_UART_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ BOOLEAN uartEnable)
{
    REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    UCHAR deviceEnabledFlags = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_ENABLES);
    if (uartEnable)
    {
        deviceEnabledFlags |= AUX_ENABLES_ENABLE_UART;
    }
    else
    {
        deviceEnabledFlags &= ~AUX_ENABLES_ENABLE_UART;
    }
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_ENABLES, deviceEnabledFlags);
}

FORCEINLINE VOID WRITE_INTERRUPT_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR interruptValue)
{
    REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IER_REGISTER, interruptValue);
}

FORCEINLINE VOID DISABLE_ALL_INTERRUPTS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, 0);
}

FORCEINLINE VOID WRITE_FIFO_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR fifoControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IIR_REGISTER, fifoControlValue);
}

FORCEINLINE UCHAR READ_LINE_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LSR_REGISTER);
}

FORCEINLINE VOID WRITE_LINE_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControlValue);
}

FORCEINLINE UCHAR READ_MODEM_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_MSR_REGISTER);
}

FORCEINLINE VOID WRITE_MODEM_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR modemControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_MCR_REGISTER, modemControlValue);
}

FORCEINLINE VOID WRITE_DIVISOR_LATCH(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT divisorValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    const UCHAR lineControl = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER);
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, (lineControl | AUX_MU_LCR_REGISTER_W_DLAB));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_LSB_REGISTER, (divisorValue & 0xff));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_MSB_REGISTER, ((divisorValue & 0xff00) >> 8));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControl);
}

FORCEINLINE UCHAR READ_RECEIVE_BUFFER(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_IO_REGISTER);
}

VOID LogLineStatusEvents(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineStatusRegister);
BOOLEAN GetCanReadFromLineStatus(_In_ WDFDEVICE device);
