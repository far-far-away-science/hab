#include "Receive.h"
#include "Receive.tmh"

ULONG EvtSerCx2PioReceiveReadBuffer(_In_ SERCX2PIORECEIVE pioReceive, _Out_ PUCHAR buffer, _In_ ULONG length)
{
    UNREFERENCED_PARAMETER(pioReceive);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(length);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
    return 0;
}

VOID EvtSerCx2PioReceiveEnableReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    UNREFERENCED_PARAMETER(pioReceive);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
}

BOOLEAN EvtSerCx2PioReceiveCancelReadyNotification(_In_ SERCX2PIORECEIVE pioReceive)
{
    UNREFERENCED_PARAMETER(pioReceive);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_RECEIVE, "%!FUNC! Exit");
    return TRUE;
}
