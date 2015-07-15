#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

DECLARE_HANDLE(REGBASE);

// size of the buffer where we store received but unread bytes
#define SERIAL_RECIVE_BUFFER_SIZE 100

typedef UCHAR(*PREAD_DEVICE_UCHAR)(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset);
typedef VOID(*PWRITE_DEVICE_UCHAR)(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset, _In_ UCHAR value);

typedef struct _UART_ERROR_COUNT
{
    LONG FifoOverrunError;
    LONG TxFifoDataLossOnD0Exit;
} UART_ERROR_COUNT;

typedef struct _UART_DEVICE_EXTENSION
{
    WDFINTERRUPT WdfInterrupt;
    WDFSPINLOCK WdfRegistersSpinLock;

    SERCX2PIOTRANSMIT WdfPioTransmit;
    SERCX2PIORECEIVE WdfPioReceive;

    BOOLEAN RegistersMapped;

    REGBASE ControllerAddress;
    ULONG ControllerMemorySpan;
    ULONG ControllerAddressSpace;

    BOOLEAN DeviceActive;

    // PL011 only uses up to 16 bits in all of it's registers
    USHORT ControlRegister;
    USHORT LineControlRegister;

    /*
    PREAD_DEVICE_UCHAR UartReadDeviceUChar;
    PWRITE_DEVICE_UCHAR UartWriteDeviceUChar;

    UCHAR LineStatus;
    UCHAR LineControl;
    UCHAR InterruptEnableRegister;
    UCHAR ModemStatus;
    UCHAR ModemControl;
    USHORT DivisorLatch;
    ULONG ClockRate;
    */

    USHORT TxFifoSize;
    USHORT RxFifoSize;

    USHORT InstanceId;

    POHANDLE PoHandle;

    UART_ERROR_COUNT ErrorCount;
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
} UART_HARDWARE_CONFIGURATION, *PUART_HARDWARE_CONFIGURATION;
