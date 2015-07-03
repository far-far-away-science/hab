#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

DECLARE_HANDLE(REGBASE);

typedef UCHAR(*PREAD_DEVICE_UCHAR)(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset);
typedef VOID(*PWRITE_DEVICE_UCHAR)(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset, _In_ UCHAR value);

typedef struct _UART_DEVICE_EXTENSION
{
    WDFDEVICE WdfDevice;

    WDFSPINLOCK WdfDpcSpinLock;
    WDFINTERRUPT WdfInterrupt;

    SERCX2PIOTRANSMIT WdfPioTransmit;
    SERCX2PIORECEIVE WdfPioReceive;

    PREAD_DEVICE_UCHAR UartReadDeviceUChar;
    PWRITE_DEVICE_UCHAR UartWriteDeviceUChar;

    BOOLEAN RegistersMapped;

    REGBASE ControllerAddress;
    ULONG ControllerMemorySpan;
    ULONG ControllerAddressSpace;

    ULONG InterruptVector;
    KIRQL InterruptLevel;
    KAFFINITY InterruptAffinity;

    BOOLEAN DeviceActive;
    UCHAR LineStatus;
    UCHAR LineControl;
    UCHAR ModemStatus;
    UCHAR ModemControl;
    USHORT DivisorLatch;
    ULONG ClockRate;

    USHORT TxFifoSize;
    USHORT RxFifoSize;
} UART_DEVICE_EXTENSION, *PUART_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(UART_DEVICE_EXTENSION, GetUartDeviceExtension);

typedef struct _SERCX2_PIO_TRANSMIT_CONTEXT
{
    WDFDEVICE WdfDevice;
} SERCX2_PIO_TRANSMIT_CONTEXT, *PSERCX2_PIO_TRANSMIT_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SERCX2_PIO_TRANSMIT_CONTEXT, GetSerCx2PioTransmitContext);

typedef struct _SERCX2_PIO_RECEIVE_CONTEXT
{
    WDFDEVICE WdfDevice;
} SERCX2_PIO_RECEIVE_CONTEXT, *PSERCX2_PIO_RECEIVE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SERCX2_PIO_RECEIVE_CONTEXT, GetSerCx2PioReceiveContext);

typedef struct _UART_HARDWARE_CONFIGURATION
{
    ULONG AddressSpace;

    PHYSICAL_ADDRESS MemoryStart;
    PHYSICAL_ADDRESS MemoryStartTranslated;
    ULONG MemoryLength;

    ULONG InterruptVector;
    ULONG InterruptLevel;
    KAFFINITY InterruptAffinity;
} UART_HARDWARE_CONFIGURATION, *PUART_HARDWARE_CONFIGURATION;