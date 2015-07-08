#include "Power.h"

#include "CommonDefinitions.h"

#include "Power.tmh"

NTSTATUS PowerEvtDeviceWdmPostPoFxRegisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    pDeviceExtension->PoHandle = poHandle;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

void PowerEvtDeviceWdmPrePoFxUnregisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    UNREFERENCED_PARAMETER(poHandle);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    pDeviceExtension->PoHandle = NULL;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
}
