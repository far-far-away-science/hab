#include "Device.h"

#include "Uart.h"
#include "Power.h"
#include "Receive.h"
#include "Transmit.h"
#include "Interrupt.h"

#include "Device.tmh"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, UartDeviceCreate)
#endif

NTSTATUS UartDeviceInitPio(_In_ WDFDEVICE device);
NTSTATUS UartDeviceInitSerCx2(_In_ WDFDEVICE device);
NTSTATUS UartDeviceInitInterrupts(_In_ WDFDEVICE device);
NTSTATUS UartDeviceInitIdleTimeout(_In_ WDFDEVICE device);

NTSTATUS UartDeviceCreate(_In_ PWDFDEVICE_INIT deviceInit)
{
    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = UartDeviceEvtPrepareHardware;
    pnpPowerCallbacks.EvtDeviceReleaseHardware = UartDeviceEvtReleaseHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = PowerEvtD0Entry;
    pnpPowerCallbacks.EvtDeviceD0EntryPostInterruptsEnabled = PowerEvtD0EntryPostInterruptsEnabled;
    pnpPowerCallbacks.EvtDeviceD0Exit = PowerEvtD0Exit;
    pnpPowerCallbacks.EvtDeviceD0ExitPreInterruptsDisabled = PowerEvtD0ExitPreInterruptsDisabled;
    WdfDeviceInitSetPnpPowerEventCallbacks(deviceInit, &pnpPowerCallbacks);

    NTSTATUS status = SerCx2InitializeDeviceInit(deviceInit);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2InitializeDeviceInit(...) failed %!STATUS!", status);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, UART_DEVICE_EXTENSION);

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

    status = UartInitContext(device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartInitContext(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitSerCx2(device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitSerCx2(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitInterrupts(device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitInterrupts(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitIdleTimeout(device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitInterrupts(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitPio(device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitPio(...) failed %!STATUS!", status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceInitSerCx2(_In_ WDFDEVICE device)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    SERCX2_CONFIG serCx2Config;

    SERCX2_CONFIG_INIT(&serCx2Config,
                       UartEvtSerCx2ApplyConfig,
                       UartEvtSerCx2Control,
                       UartEvtSerCx2PurgeFifos);

    serCx2Config.EvtSerCx2SetWaitMask = UartEvtSerCx2SetWaitMask;

    NTSTATUS status = SerCx2InitializeDevice(device, &serCx2Config);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2InitializeDevice(...) failed %!STATUS!", status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceInitInterrupts(_In_ WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    // spin lock for interriupt DPC
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = device;

    NTSTATUS status = WdfSpinLockCreate(&attributes, &pDeviceExtension->WdfDpcSpinLock);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfSpinLockCreate(...) DPC failed %!STATUS!", status);
        return status;
    }

    // spinlock for interrupt
    WDFSPINLOCK interruptLock;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = device;

    status = WdfSpinLockCreate(&attributes, &interruptLock);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfSpinLockCreate(...) failed %!STATUS!", status);
        return status;
    }

    // setip interrupt
    WDF_INTERRUPT_CONFIG interruptConfig;
    WDF_INTERRUPT_CONFIG_INIT(&interruptConfig, UartInterruptISR, UartInterruptTxRxDPCForISR);
    interruptConfig.SpinLock = interruptLock;
    interruptConfig.EvtInterruptEnable = UartInterruptEvtInterruptEnable;
    interruptConfig.EvtInterruptDisable = UartInterruptEvtInterruptDisable;

    status = WdfInterruptCreate(device, &interruptConfig, WDF_NO_OBJECT_ATTRIBUTES, &pDeviceExtension->WdfInterrupt);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfInterruptCreate(...) failed %!STATUS!", status);
        return status;
    }

    WDF_INTERRUPT_EXTENDED_POLICY policyAndGroup;
    WDF_INTERRUPT_EXTENDED_POLICY_INIT(&policyAndGroup);
    policyAndGroup.Priority = WdfIrqPriorityNormal;
    WdfInterruptSetExtendedPolicy(pDeviceExtension->WdfInterrupt, &policyAndGroup);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceInitIdleTimeout(_In_ WDFDEVICE device)
{
    UNREFERENCED_PARAMETER(device);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDF_POWER_FRAMEWORK_SETTINGS powerFrameworkSettings;
    WDF_POWER_FRAMEWORK_SETTINGS_INIT(&powerFrameworkSettings);
    powerFrameworkSettings.EvtDeviceWdmPostPoFxRegisterDevice = PowerEvtDeviceWdmPostPoFxRegisterDevice;
    powerFrameworkSettings.EvtDeviceWdmPrePoFxUnregisterDevice = PowerEvtDeviceWdmPrePoFxUnregisterDevice;

    NTSTATUS status = WdfDeviceWdmAssignPowerFrameworkSettings(device, &powerFrameworkSettings);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceWdmAssignPowerFrameworkSettings(...) failed %!STATUS!", status);
        return status;
    }

    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleCannotWakeFromS0);

    idleSettings.IdleTimeout = 100; // ms TODO need to check what value is good for 4800 baud
    idleSettings.IdleTimeoutType = SystemManagedIdleTimeoutWithHint;

    status = WdfDeviceAssignS0IdleSettings(device, &idleSettings);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceAssignS0IdleSettings(...) failed %!STATUS!", status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceInitPio(_In_ WDFDEVICE device)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, SERCX2_PIO_TRANSMIT_CONTEXT);

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    SERCX2_PIO_TRANSMIT_CONFIG transmitConfig;
    SERCX2_PIO_TRANSMIT_CONFIG_INIT(&transmitConfig,
                                    EvtSerCx2PioTransmitWriteBuffer,
                                    EvtSerCx2PioTransmitEnableReadyNotification,
                                    EvtSerCx2PioTransmitCancelReadyNotification);
    NTSTATUS status = SerCx2PioTransmitCreate(device, &transmitConfig, &attributes, &pDeviceExtension->WdfPioTransmit);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2PioTransmitCreate(...) failed %!STATUS!", status);
        return status;
    }

    PSERCX2_PIO_TRANSMIT_CONTEXT pTransmitContext = GetSerCx2PioTransmitContext(pDeviceExtension->WdfPioTransmit);
    pTransmitContext->WdfDevice = device;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, SERCX2_PIO_RECEIVE_CONTEXT);

    SERCX2_PIO_RECEIVE_CONFIG receiveConfig;
    SERCX2_PIO_RECEIVE_CONFIG_INIT(&receiveConfig,
                                   EvtSerCx2PioReceiveReadBuffer,
                                   EvtSerCx2PioReceiveEnableReadyNotification,
                                   EvtSerCx2PioReceiveCancelReadyNotification);

    status = SerCx2PioReceiveCreate(device, &receiveConfig, &attributes, &pDeviceExtension->WdfPioReceive);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2PioTransmitCreate(...) failed %!STATUS!", status);
        return status;
    }

    PSERCX2_PIO_RECEIVE_CONTEXT pReceiveContext = GetSerCx2PioReceiveContext(pDeviceExtension->WdfPioReceive);
    pReceiveContext->WdfDevice = device;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtPrepareHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resources, _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resources);
    UNREFERENCED_PARAMETER(resourcesTranslated);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO map hardware resources
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtReleaseHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(device);
    UNREFERENCED_PARAMETER(resourcesTranslated);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");
    // TODO unmap memory mapped registers
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}
