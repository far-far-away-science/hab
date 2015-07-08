#include "Receive.h"

#include "DeviceDefinitions.h"

#include "Receive.tmh"

ULONG EvtSerCx2PioReceiveReadBuffer(_In_ SERCX2PIORECEIVE pioReceive, _Out_ PUCHAR buffer, _In_ ULONG length)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);

    ULONG bytesTransferred = 0;
    for (; CanReadCharacter(pDeviceExtension) && bytesTransferred < length; ++bytesTransferred)
    {
        buffer[bytesTransferred] = RECEIVE_BUFFER_READ(pDeviceExtension);
    }

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit (bytes transmitted = %lu)", bytesTransferred);
    return bytesTransferred;
}

VOID EvtSerCx2PioReceiveEnableReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    UNREFERENCED_PARAMETER(pioReceive);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);
    INTERRUPT_ENABLE_RECEIVE_DATA_AVAILABLE(pDeviceExtension);
    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
}

BOOLEAN EvtSerCx2PioReceiveCancelReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    UNREFERENCED_PARAMETER(pioReceive);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");

    PSERCX2_PIO_RECEIVE_CONTEXT pSerCx2Context = GetSerCx2PioReceiveContext(pioReceive);
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(pSerCx2Context->WdfDevice);

    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);
    BOOLEAN cancelHandledSuccessfully = FALSE;
    const UCHAR receiveInterrupts = INTERRUPT_READ_MASKED_RECEIVE_DATA_AVAILABLE(pDeviceExtension);
    if (receiveInterrupts != 0)
    {
        cancelHandledSuccessfully = TRUE;
        INTERRUPT_DISABLE_RECEIVE_DATA_AVAILABLE(pDeviceExtension);
    }
    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
    return cancelHandledSuccessfully;
}

UCHAR UartReadRegisterUChar(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset)
{
    return READ_REGISTER_UCHAR(((PUCHAR) baseAddress) + offset);
}
