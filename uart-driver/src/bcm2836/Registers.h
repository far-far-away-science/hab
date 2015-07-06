#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\CommonDefinitions.h"

#include "..\..\Trace.h"

#define AUX_ENABLES 0x04
    #define AUX_ENABLES_ENABLE_UART (1 << 0) // if 1 uart device is enabled

#define AUX_MU_IO_REGISTER 0x40 // I/O register

#define AUX_MU_IER_REGISTER 0x44 // interrupt enable register
    #define AUX_MU_IER_REGISTER_RW_RLS (1 << 2) // receiver line status register change

#define AUX_MU_IIR_REGISTER 0x48 // interrupt identity register
    #define AUX_MU_IIR_REGISTER_W_RCVR_RESET (1 << 1) // write 1 to reset receive FIFO
    #define AUX_MU_IIR_REGISTER_W_TXMT_RESET (1 << 2) // write 1 to reset transmit FIFO

#define AUX_MU_LCR_REGISTER 0x4C // line control register
    #define AUX_MU_LCR_REGISTER_RW_DLAB (1 << 7)

#define AUX_MU_MCR_REGISTER 0x50 // modem control register
    #define AUX_MU_MCR_REGISTER_RW_RTS (1 << 2) // request to send

#define AUX_MU_LSR_REGISTER 0x54 // line status register
    #define AUX_MU_LSR_REGISTER_R_OE   (1 << 1) // Overrun error
    #define AUX_MU_LSR_REGISTER_R_PE   (1 << 2) // Parity error

    #define AUX_MU_LSR_REGISTER_R_THRE (1 << 5) // THR (transmitter holding register) is empty
    #define AUX_MU_LSR_REGISTER_R_TEMT (1 << 6) // THR (transmitter holding register) is empty and line is idle

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
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, (lineControl | AUX_MU_LCR_REGISTER_RW_DLAB));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_LSB_REGISTER, (divisorValue & 0xff));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_MSB_REGISTER, ((divisorValue & 0xff00) >> 8));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControl);
}

FORCEINLINE VOID DISABLE_ALL_INTERRUPTS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, 0);
}
