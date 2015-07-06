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

    // FIFO is always enabled on BCM2836 so we only need to reset it

    UCHAR fifoControlRegister = (UCHAR)
    (
        AUX_MU_IIR_REGISTER_W_RCVR_RESET |
        AUX_MU_IIR_REGISTER_W_TXMT_RESET
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

NTSTATUS PowerEvtD0Entry(_In_ WDFDEVICE device, _In_ WDF_POWER_DEVICE_STATE previousState)
{
    UNREFERENCED_PARAMETER(previousState);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    pDeviceExtension->DeviceActive = TRUE;

    WdfSpinLockAcquire(pDeviceExtension->WdfDeviceSpinLock);
    WRITE_LINE_CONTROL(pDeviceExtension, pDeviceExtension->LineControl);
    WRITE_DIVISOR_LATCH(pDeviceExtension, pDeviceExtension->DivisorLatch);
    WRITE_MODEM_CONTROL(pDeviceExtension, pDeviceExtension->ModemControl);

    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_BCM_2836_CONTROLLER,
                "restored modem to active state (line control=0x%x, divisor latch=0x%x, modem control=0x%x)", 
                (unsigned int) pDeviceExtension->LineControl,
                (unsigned int) pDeviceExtension->DivisorLatch,
                (unsigned int) pDeviceExtension->ModemControl);

    WdfSpinLockRelease(pDeviceExtension->WdfDeviceSpinLock);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS UartInterruptEvtInterruptEnable(_In_ WDFINTERRUPT interrupt, _In_ WDFDEVICE associatedDevice)
{
    UNREFERENCED_PARAMETER(interrupt);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(associatedDevice);

    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_RW_RLS;
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}
