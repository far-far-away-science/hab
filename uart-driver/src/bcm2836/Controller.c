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
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "failed to map device memory to virtual memory");
        pDeviceExtension->RegistersMapped = FALSE;
        return STATUS_NONE_MAPPED;
    }

    pDeviceExtension->ControllerAddressSpace = pUartHardwareConfiguration->AddressSpace;
    pDeviceExtension->ControllerMemorySpan = pUartHardwareConfiguration->MemoryLength;

    pDeviceExtension->InterruptVector = pUartHardwareConfiguration->InterruptVector;
    pDeviceExtension->InterruptLevel = (KIRQL)pUartHardwareConfiguration->InterruptLevel;
    pDeviceExtension->InterruptAffinity = pUartHardwareConfiguration->InterruptAffinity;

    // FIFO is always enabled on BCM2836

    UCHAR fifoControlRegister = (UCHAR)
    (
        AUX_MU_IIR_REGISTER_W_RCVR_RESET |
        AUX_MU_IIR_REGISTER_W_TXMT_RESET
    );

    WRITE_FIFO_CONTROL(pDeviceExtension, fifoControlRegister);

    pDeviceExtension->LineStatus = READ_LINE_STATUS(pDeviceExtension);
    pDeviceExtension->ModemStatus = READ_MODEM_STATUS(pDeviceExtension);

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
