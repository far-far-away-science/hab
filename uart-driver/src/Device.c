#include "Device.h"

#include "Device.tmh"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, UartDeviceCreate)
#endif

NTSTATUS UartDeviceCreate(_In_ PWDFDEVICE_INIT deviceInit)
{
    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = UartDeviceEvtPrepareHardware;
    pnpPowerCallbacks.EvtDeviceReleaseHardware = UartDeviceEvtReleaseHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = UartDeviceEvtD0Entry;
    pnpPowerCallbacks.EvtDeviceD0EntryPostInterruptsEnabled = UartDeviceEvtD0EntryPostInterruptsEnabled;
    pnpPowerCallbacks.EvtDeviceD0Exit = UartDeviceEvtD0Exit;
    pnpPowerCallbacks.EvtDeviceD0ExitPreInterruptsDisabled = UartDeviceEvtD0ExitPreInterruptsDisabled;
    WdfDeviceInitSetPnpPowerEventCallbacks(deviceInit, &pnpPowerCallbacks);

    NTSTATUS status = SerCx2InitializeDeviceInit(deviceInit);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2InitializeDeviceInit(...) failed %!STATUS!", status);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, UART_DEVICE_EXTENSION);
    attributes.EvtDestroyCallback = UartDeviceEvtDestroy;

    WDFDEVICE device = NULL;
    status = WdfDeviceCreate(&deviceInit, &attributes, &device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceCreate(...) failed %!STATUS!", status);
        return status;
    }

    WDF_DEVICE_STATE deviceState;
    WDF_DEVICE_STATE_INIT(&deviceState);
    deviceState.NotDisableable = WdfFalse;
    WdfDeviceSetDeviceState(device, &deviceState);

    // TODO initialize context
    // TODO initialize SERCX2
    // TODO initialize interrupts

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtPrepareHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resources, _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resources);
    UNREFERENCED_PARAMETER(resourcesTranslated);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtReleaseHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resourcesTranslated);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtD0Entry(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(previousState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtD0Exit(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtD0EntryPostInterruptsEnabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(previousState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtD0ExitPreInterruptsDisabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

void UartDeviceEvtDestroy(_In_ WDFOBJECT device)
{
    UNREFERENCED_PARAMETER(device);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
}
