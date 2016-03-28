#include "SerCx2Utils.h"

#include "CommonDefinitions.h"

#include "SerCx2Utils.tmh"

NTSTATUS UartEvtSerCx2Control(_In_ WDFDEVICE device,
                              _In_ WDFREQUEST request,
                              _In_ size_t outputBufferLength,
                              _In_ size_t inputBufferLength,
                              _In_ ULONG ioControlCode)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(request);
    UNREFERENCED_PARAMETER(outputBufferLength);
    UNREFERENCED_PARAMETER(inputBufferLength);
    UNREFERENCED_PARAMETER(ioControlCode);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID UartEvtSerCx2PurgeFifos(_In_ WDFDEVICE device,
                             _In_ BOOLEAN purgeRxFifo,
                             _In_ BOOLEAN purgeTxFifo)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(purgeRxFifo);
    UNREFERENCED_PARAMETER(purgeTxFifo);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Exit");
}

NTSTATUS UartEvtSerCx2ApplyConfig(_In_ WDFDEVICE device,
                                  _In_ PVOID connectionParameters)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(connectionParameters);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID UartEvtSerCx2SetWaitMask(_In_ WDFDEVICE device,
                              _In_ WDFREQUEST request,
                              _In_ ULONG waitMask)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(request);
    UNREFERENCED_PARAMETER(waitMask);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_SERCX2, "--- %!FUNC! Exit");
}
