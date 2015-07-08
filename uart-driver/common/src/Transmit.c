#include "Transmit.h"
#include "Transmit.tmh"

ULONG EvtSerCx2PioTransmitWriteBuffer(_In_ SERCX2PIOTRANSMIT pioTransmit, _In_ PUCHAR buffer, _In_ ULONG length)
{
    UNREFERENCED_PARAMETER(pioTransmit);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(length);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Entry");

    // TODO

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Exit");
    return 0;
}

VOID EvtSerCx2PioTransmitEnableReadyNotification(_In_ SERCX2PIOTRANSMIT pioTransmit)
{
    UNREFERENCED_PARAMETER(pioTransmit);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Entry");

    // TODO

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Exit");
}

BOOLEAN EvtSerCx2PioTransmitCancelReadyNotification(_In_ SERCX2PIOTRANSMIT pioTransmit)
{
    UNREFERENCED_PARAMETER(pioTransmit);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Entry");

    // TODO

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_TRANSMIT, "--- %!FUNC! Exit");
    return TRUE;
}

VOID UartWriteRegisterUChar(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset, _In_ UCHAR value)
{
    WRITE_REGISTER_UCHAR(((PUCHAR) baseAddress) + offset, value);
}
