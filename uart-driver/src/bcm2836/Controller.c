#include "Controller.h"
#include "Controller.tmh"

#include "Registers.h"
#include "bcm2836Definitions.h"

NTSTATUS InitializeUartController(_In_ WDFDEVICE device, _In_ const UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Entry");

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
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BCM_2836_CONTROLLER, "failed to map device memory to virtual memory");
        pDeviceExtension->RegistersMapped = FALSE;
        return STATUS_NONE_MAPPED;
    }

    pDeviceExtension->ControllerAddressSpace = pUartHardwareConfiguration->AddressSpace;
    pDeviceExtension->ControllerMemorySpan = pUartHardwareConfiguration->MemoryLength;

    pDeviceExtension->InterruptVector = pUartHardwareConfiguration->InterruptVector;
    pDeviceExtension->InterruptLevel = (KIRQL) pUartHardwareConfiguration->InterruptLevel;
    pDeviceExtension->InterruptAffinity = pUartHardwareConfiguration->InterruptAffinity;

    WRITE_UART_ENABLE(pDeviceExtension, TRUE);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "enabled uart device");

    // FIFO is always enabled on BCM2836 so we only need to reset it

    UCHAR fifoControlRegister = (UCHAR)
    (
        AUX_MU_IIR_REGISTER_W_RESET_RECEIVE_FIFO |
        AUX_MU_IIR_REGISTER_W_RESET_TRANSMIT_FIFO
    );

    WRITE_FIFO_CONTROL(pDeviceExtension, fifoControlRegister);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "wrote FIFO control UCHAR 0x%x", (unsigned int) fifoControlRegister);

    pDeviceExtension->LineStatus = READ_LINE_STATUS(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "read line status UCHAR 0x%x", (unsigned int) pDeviceExtension->LineStatus);

    pDeviceExtension->ModemStatus = READ_MODEM_STATUS(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "read modem status UCHAR 0x%x", (unsigned int) pDeviceExtension->ModemStatus);

    pDeviceExtension->ModemControl = 0;
    pDeviceExtension->LineControl = 0;
    pDeviceExtension->DivisorLatch = 0;
    pDeviceExtension->ClockRate = 0;
    pDeviceExtension->DeviceActive = FALSE;

    pDeviceExtension->TxFifoSize = SERIAL_TX_FIFO_SIZE_DEFAULT;
    pDeviceExtension->RxFifoSize = SERIAL_RX_FIFO_SIZE_DEFAULT;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID UninitializeUartController(_In_ WDFDEVICE device)
{
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    WRITE_UART_ENABLE(pDeviceExtension, FALSE);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "disabled uart device");
}

NTSTATUS PowerEvtD0Entry(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(previousState);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    pDeviceExtension->DeviceActive = TRUE;

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);
    WRITE_LINE_CONTROL(pDeviceExtension, pDeviceExtension->LineControl);
    WRITE_DIVISOR_LATCH(pDeviceExtension, pDeviceExtension->DivisorLatch);
    WRITE_MODEM_CONTROL(pDeviceExtension, pDeviceExtension->ModemControl);

    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_BCM_2836_CONTROLLER,
                "restored modem to active state (line control=0x%x, divisor latch=0x%x, modem control=0x%x)", 
                (unsigned int) pDeviceExtension->LineControl,
                (unsigned int) pDeviceExtension->DivisorLatch,
                (unsigned int) pDeviceExtension->ModemControl);

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartInterruptEvtInterruptEnable(_In_ WDFINTERRUPT interrupt, _In_ WDFDEVICE associatedDevice)
{
    UNREFERENCED_PARAMETER(interrupt);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(associatedDevice);

    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0ExitPreInterruptsDisabled(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    pDeviceExtension->DeviceActive = FALSE;

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);

    WRITE_MODEM_CONTROL(pDeviceExtension, pDeviceExtension->ModemControl & ~AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE);

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    SerCx2SaveReceiveFifoOnD0Exit(pDeviceExtension->WdfPioReceive, SERIAL_RECIVE_BUFFER_SIZE);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS PowerEvtD0Exit(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE targetState)
{
    UNREFERENCED_PARAMETER(targetState);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    WdfSpinLockAcquire(pDeviceExtension->WdfRegistersSpinLock);

    const UCHAR lineStatusRegister = READ_LINE_STATUS(pDeviceExtension);
    LogLineStatusEvents(pDeviceExtension, lineStatusRegister);

    if (((lineStatusRegister & AUX_MU_LSR_REGISTER_R_TRANSMITTER_EMPTY) == 0) || ((lineStatusRegister & AUX_MU_LSR_REGISTER_R_TRANSMITTER_IDLE) == 0))
    {
        // some data is still available in transmitter
        const long int newValue = InterlockedIncrement(&pDeviceExtension->ErrorCount.TxFifoDataLossOnD0Exit);
        // TODO LOG in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BCM_2836_CONTROLLER, "data loss on EvtD0Exit (count = %li)", (long int) newValue);
    }

    WdfSpinLockRelease(pDeviceExtension->WdfRegistersSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}
