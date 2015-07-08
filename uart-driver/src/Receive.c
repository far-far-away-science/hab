#include "Receive.h"

#include "bcm2836\Registers.h"

#include "Receive.tmh"

ULONG EvtSerCx2PioReceiveReadBuffer(_In_ SERCX2PIORECEIVE pioReceive, _Out_ PUCHAR buffer, _In_ ULONG length)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);

    ULONG bytesTransferred = 0;
    BOOLEAN canRead = GetCanReadFromLineStatus(pSerCx2Context->WdfDevice);

    for (; canRead && bytesTransferred < length; ++bytesTransferred)
    {
        buffer[bytesTransferred] = READ_RECEIVE_BUFFER(pDeviceExtension);
        canRead = GetCanReadFromLineStatus(pSerCx2Context->WdfDevice);
    }

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit (bytes transmitted = %lu)", bytesTransferred);
    return bytesTransferred;
}

VOID EvtSerCx2PioReceiveEnableReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);
    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
}

BOOLEAN EvtSerCx2PioReceiveCancelReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);

    BOOLEAN interruptsEnabled = (pDeviceExtension->InterruptEnableRegister & AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE) != 0;

    if (interruptsEnabled)
    {
        pDeviceExtension->InterruptEnableRegister &= ~AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
        WRITE_INTERRUPT_ENABLE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
    }

    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
    return interruptsEnabled;
}

UCHAR UartReadRegisterUChar(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset)
{
    const UCHAR readChar = READ_REGISTER_UCHAR(((PUCHAR) baseAddress) + offset);
    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_RECEIVE,
                "read register UCHAR (0x%p + 0x%lx) = 0x%x",
                (void*) baseAddress,
                (long int) offset,
                (unsigned int) readChar);
    return readChar;
}
