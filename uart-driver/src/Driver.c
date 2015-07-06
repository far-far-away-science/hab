#include "Driver.h"

#include "Device.h"

#include "Driver.tmh"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, DriverEntry)
    #pragma alloc_text (PAGE, UartDriverEvtDeviceAdd)
    #pragma alloc_text (PAGE, UartDriverEvtDriverContextCleanup)
#endif

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath)
{
    WPP_INIT_TRACING(driverObject, registryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = UartDriverEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config, UartDriverEvtDeviceAdd);

    NTSTATUS status = WdfDriverCreate(driverObject, registryPath, &attributes, &config, WDF_NO_HANDLE);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(driverObject);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS UartDriverEvtDeviceAdd(_In_ WDFDRIVER driver, _Inout_ PWDFDEVICE_INIT deviceInit)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(driver);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    NTSTATUS status = UartDeviceCreate(deviceInit);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

void UartDriverEvtDriverContextCleanup(_In_ WDFOBJECT driverObject)
{
    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "(trace shuts down after this message and this is a last entry) %!FUNC! Entry");

    // Stop WPP Tracing
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER) driverObject));
}
