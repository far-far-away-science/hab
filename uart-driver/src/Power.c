#include "Power.h"
#include "Power.tmh"

NTSTATUS PowerEvtD0Entry(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(previousState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0EntryPostInterruptsEnabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(previousState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0Exit(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0ExitPreInterruptsDisabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtDeviceWdmPostPoFxRegisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(poHandle);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

void PowerEvtDeviceWdmPrePoFxUnregisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(poHandle);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
}
