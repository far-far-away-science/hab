#include "Device.h"

#include "Power.h"
#include "Receive.h"
#include "Transmit.h"
#include "Interrupt.h"
#include "SerCx2Utils.h"
#include "DeviceSpecific.h"

#include "..\..\Trace.h"

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
    pnpPowerCallbacks.EvtDeviceD0ExitPreInterruptsDisabled = PowerEvtD0ExitPreInterruptsDisabled;
    pnpPowerCallbacks.EvtDeviceD0Exit = PowerEvtD0Exit;
    WdfDeviceInitSetPnpPowerEventCallbacks(deviceInit, &pnpPowerCallbacks);

    NTSTATUS status = SerCx2InitializeDeviceInit(deviceInit);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2InitializeDeviceInit(...) failed %!STATUS!", status);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, UART_DEVICE_EXTENSION);

    WDFDEVICE device = NULL;
    status = WdfDeviceCreate(&deviceInit, &attributes, &device);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceCreate(...) failed %!STATUS!", status);
        return status;
    }

    WDF_DEVICE_STATE deviceState;
    WDF_DEVICE_STATE_INIT(&deviceState);
    deviceState.NotDisableable = WdfFalse;
    WdfDeviceSetDeviceState(device, &deviceState);

    status = UartDeviceInitSerCx2(device);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitSerCx2(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitInterrupts(device);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitInterrupts(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitIdleTimeout(device);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "UartDeviceInitIdleTimeout(...) failed %!STATUS!", status);
        return status;
    }

    status = UartDeviceInitPio(device);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
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
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2InitializeDevice(...) failed %!STATUS!", status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS QueryId(_In_ WDFDEVICE device, _Out_ USHORT* pInstanceId)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDFIOTARGET ioTarget = WdfDeviceGetIoTarget(device);

    if (ioTarget == NULL)
    {
        // TODO lot in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceGetIoTarget(...) returned nullptr io target");
        return STATUS_UNSUCCESSFUL;
    }

    WDFREQUEST request = NULL;
    NTSTATUS status = WdfRequestCreate(WDF_NO_OBJECT_ATTRIBUTES, ioTarget, &request);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfRequestCreate(...) failed %!STATUS!", status);
        return status;
    }

    WDF_REQUEST_REUSE_PARAMS reuseParams;
    WDF_REQUEST_REUSE_PARAMS_INIT(&reuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_NOT_SUPPORTED);

    status = WdfRequestReuse(request, &reuseParams);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfRequestReuse(...) failed %!STATUS!", status);
        return status;
    }

    IO_STACK_LOCATION ioStack;
    RtlZeroMemory(&ioStack, sizeof(ioStack));
    ioStack.MinorFunction = IRP_MN_QUERY_ID;
    ioStack.MajorFunction = IRP_MJ_PNP;
    ioStack.Parameters.QueryId.IdType = BusQueryInstanceID;
    WdfRequestWdmFormatUsingStackLocation(request, &ioStack);

    WDF_REQUEST_SEND_OPTIONS sendOptions;
    WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);

    if (!WdfRequestSend(request, ioTarget, &sendOptions))
    {
        // TODO lot in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfRequestSend(...) failed");
        return STATUS_UNSUCCESSFUL;
    }

    status = WdfRequestGetStatus(request);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfRequestGetStatus(...) failed %!STATUS!", status);
        return status;
    }

    UNICODE_STRING destinationString;
    PWCHAR pInstanceIdString = (PWCHAR) WdfRequestGetInformation(request);
    RtlInitUnicodeString(&destinationString, pInstanceIdString);

    if (destinationString.Length == 0)
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfRequestGetInformation(...) failed to get string id");
        return STATUS_INVALID_PARAMETER;
    }

    ULONG instanceId;
    status = RtlUnicodeStringToInteger(&destinationString, 10, &instanceId);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "RtlUnicodeStringToInteger(...) failed %!STATUS!", status);
        return status;
    }

    *pInstanceId = (USHORT) instanceId;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceInitInterrupts(_In_ WDFDEVICE device)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    // spin lock for registers
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = device;

    NTSTATUS status = WdfSpinLockCreate(&attributes, &pDeviceExtension->WdfRegistersSpinLock);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfSpinLockCreate(...) registers failed %!STATUS!", status);
        return status;
    }

    // spinlock for interrupt
    WDFSPINLOCK interruptLock;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = device;

    status = WdfSpinLockCreate(&attributes, &interruptLock);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfSpinLockCreate(...) interrupt failed %!STATUS!", status);
        return status;
    }

    // setup interrupt
    WDF_INTERRUPT_CONFIG interruptConfig;
    WDF_INTERRUPT_CONFIG_INIT(&interruptConfig, UartInterruptISR, UartInterruptTxRxDPCForISR);
    interruptConfig.SpinLock = interruptLock;
    interruptConfig.EvtInterruptEnable = UartInterruptEvtInterruptEnable;
    interruptConfig.EvtInterruptDisable = UartInterruptEvtInterruptDisable;

    status = WdfInterruptCreate(device, &interruptConfig, WDF_NO_OBJECT_ATTRIBUTES, &pDeviceExtension->WdfInterrupt);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfInterruptCreate(...) failed %!STATUS!", status);
        return status;
    }

    status = QueryId(device, &pDeviceExtension->InstanceId);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "QueryId(...) failed %!STATUS!", status);
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
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    WDF_POWER_FRAMEWORK_SETTINGS powerFrameworkSettings;
    WDF_POWER_FRAMEWORK_SETTINGS_INIT(&powerFrameworkSettings);
    powerFrameworkSettings.EvtDeviceWdmPostPoFxRegisterDevice = PowerEvtDeviceWdmPostPoFxRegisterDevice;
    powerFrameworkSettings.EvtDeviceWdmPrePoFxUnregisterDevice = PowerEvtDeviceWdmPrePoFxUnregisterDevice;

    NTSTATUS status = WdfDeviceWdmAssignPowerFrameworkSettings(device, &powerFrameworkSettings);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "WdfDeviceWdmAssignPowerFrameworkSettings(...) failed %!STATUS!", status);
        return status;
    }

    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleCannotWakeFromS0);

    idleSettings.IdleTimeout = 5000; // ms TODO fake for now, need to check what value is good for 1200 baud
    idleSettings.IdleTimeoutType = SystemManagedIdleTimeoutWithHint;

    status = WdfDeviceAssignS0IdleSettings(device, &idleSettings);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
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
        // TODO log in ETW
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
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "SerCx2PioTransmitCreate(...) failed %!STATUS!", status);
        return status;
    }

    PSERCX2_PIO_RECEIVE_CONTEXT pReceiveContext = GetSerCx2PioReceiveContext(pDeviceExtension->WdfPioReceive);
    pReceiveContext->WdfDevice = device;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS PrepareUartHardware(_In_ WDFCMRESLIST resources, _In_ WDFCMRESLIST resourcesTranslated, _In_ UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    const ULONG resourceListCount = resources ? WdfCmResourceListGetCount(resources) : 0;
    const ULONG resourceTranslatedListCount = resourcesTranslated ? WdfCmResourceListGetCount(resourcesTranslated) : 0;

    if (resourceListCount != resourceTranslatedListCount)
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "resourceListCountRaw != resourceListCountTrans");
        return STATUS_UNSUCCESSFUL;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "number of resources = %lu", (unsigned long) resourceListCount);

    ULONG numberOfMemoryResourcesFound = 0;

    for (ULONG i = 0; i < resourceListCount; i++)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceDesc = WdfCmResourceListGetDescriptor(resources, i);
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialResourceTransltedDesc = WdfCmResourceListGetDescriptor(resourcesTranslated, i);

        if (pPartialResourceDesc && pPartialResourceTransltedDesc)
        {
            switch (pPartialResourceTransltedDesc->Type)
            {
                case CmResourceTypeMemory:
                {
                    if (numberOfMemoryResourcesFound == 0)
                    {
                        pUartHardwareConfiguration->MemoryStart = pPartialResourceDesc->u.Memory.Start;
                        pUartHardwareConfiguration->MemoryStartTranslated = pPartialResourceTransltedDesc->u.Memory.Start;
                        pUartHardwareConfiguration->MemoryLength = pPartialResourceDesc->u.Memory.Length;
                        pUartHardwareConfiguration->AddressSpace = CM_RESOURCE_PORT_MEMORY;
                        TraceEvents(TRACE_LEVEL_INFORMATION,
                                    TRACE_DEVICE,
                                    "memory resource found (start=0x%llx,startTranslated=0x%llx,length=0x%lx)",
                                    (unsigned long long) pUartHardwareConfiguration->MemoryStart.QuadPart,
                                    (unsigned long long) pUartHardwareConfiguration->MemoryStartTranslated.QuadPart,
                                    (unsigned long) pUartHardwareConfiguration->MemoryLength);
                    }
                    else
                    {
                        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "found more than one memory resource, not sure what to do about them");
                    }
                    ++numberOfMemoryResourcesFound;
                    break;
                }
                default:
                {
                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "found unsupported resource with type = %hu", (unsigned short int) pPartialResourceTransltedDesc->Type);
                }
            }
        }
        else
        {
            // TODO log in ETW
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "invalid arguments pPartialResourceDesc=0x%p, pPartialResourceTransltedDesc=0x%p", pPartialResourceDesc, pPartialResourceTransltedDesc);
        }
    }

    if (numberOfMemoryResourcesFound == 0)
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "didn't find any memory resources");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS InitializeUartController(_In_ WDFDEVICE device, _In_ const UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    if (pUartHardwareConfiguration->AddressSpace == 0)
    {
        pDeviceExtension->ControllerAddress = MmMapIoSpace(pUartHardwareConfiguration->MemoryStartTranslated, pUartHardwareConfiguration->MemoryLength, MmNonCached);
        pDeviceExtension->RegistersMapped = pDeviceExtension->ControllerAddress != NULL;
    }
    else
    {
        pDeviceExtension->ControllerAddress = ULongToPtr(pUartHardwareConfiguration->MemoryStartTranslated.LowPart);
        pDeviceExtension->RegistersMapped = FALSE;
    }

    if (!pDeviceExtension->ControllerAddress)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "failed to map device memory to virtual memory");
        pDeviceExtension->RegistersMapped = FALSE;
        return STATUS_NONE_MAPPED;
    }

    pDeviceExtension->ControllerAddressSpace = pUartHardwareConfiguration->AddressSpace;
    pDeviceExtension->ControllerMemorySpan = pUartHardwareConfiguration->MemoryLength;

    UART_DEVICE_ENABLE(pDeviceExtension, TRUE);
    INITIALIZE_AND_BACKUP_REGISTERS_IN_MEMORY(pDeviceExtension);
    FIFO_CONTROL_ENABLE(pDeviceExtension);

    pDeviceExtension->DeviceActive = FALSE;

    pDeviceExtension->TxFifoSize = READ_SERIAL_TX_FIFO_SIZE();
    pDeviceExtension->RxFifoSize = READ_SERIAL_RX_FIFO_SIZE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtPrepareHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resources, _In_ WDFCMRESLIST resourcesTranslated)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    UART_HARDWARE_CONFIGURATION uartHardwareConfiguration = { 0 };
    NTSTATUS status = PrepareUartHardware(resources, resourcesTranslated, &uartHardwareConfiguration);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "PrepareUartHardware(...) failed %!STATUS!", status);
        return status;
    }

    status = InitializeUartController(device, &uartHardwareConfiguration);

    if (!NT_SUCCESS(status))
    {
        // TODO log in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "InitializeUartController(...) failed %!STATUS!", status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartDeviceEvtReleaseHardware(_In_ WDFDEVICE device, _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(resourcesTranslated);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    if (pDeviceExtension->RegistersMapped)
    {
        UART_DEVICE_ENABLE(pDeviceExtension, FALSE);

        MmUnmapIoSpace(pDeviceExtension->ControllerAddress, pDeviceExtension->ControllerMemorySpan);

        pDeviceExtension->ControllerAddress = NULL;
        pDeviceExtension->ControllerMemorySpan = 0;
        pDeviceExtension->RegistersMapped = FALSE;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}
