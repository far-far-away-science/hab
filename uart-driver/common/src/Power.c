#include "Power.h"

#include "..\DeviceDefinitions.h"

#include "Power.tmh"

NTSTATUS PowerEvtD0Entry(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(previousState);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    pDeviceExtension->DeviceActive = TRUE;

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);
    WRITE_LINE_CONTROL(pDeviceExtension, pDeviceExtension->LineControl);
    WRITE_DIVISOR_LATCH(pDeviceExtension, pDeviceExtension->DivisorLatch);
    WRITE_MODEM_CONTROL(pDeviceExtension, pDeviceExtension->ModemControl);

    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_POWER,
                "restored modem to active state (line control=0x%x, divisor latch=0x%x, modem control=0x%x)",
                (unsigned int)pDeviceExtension->LineControl,
                (unsigned int)pDeviceExtension->DivisorLatch,
                (unsigned int)pDeviceExtension->ModemControl);

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtDeviceWdmPostPoFxRegisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    pDeviceExtension->PoHandle = poHandle;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

void PowerEvtDeviceWdmPrePoFxUnregisterDevice(_In_ WDFDEVICE device, _In_ POHANDLE poHandle)
{
    UNREFERENCED_PARAMETER(poHandle);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    pDeviceExtension->PoHandle = NULL;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER, "%!FUNC! Exit");
}

NTSTATUS PowerEvtD0ExitPreInterruptsDisabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "--- %!FUNC! Entry");

    // TODO

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "--- %!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0Exit(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "--- %!FUNC! Entry");

    // TODO

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "--- %!FUNC! Exit");
    return STATUS_SUCCESS;
}
