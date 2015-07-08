#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "Registers.h"
#include "Definitions.h"

#include "..\..\Trace.h"

#include "Controller.tmh"

NTSTATUS InitializeUartController(_In_ WDFDEVICE device, _In_ const UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Entry");

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
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_UART_MINI_CONTROLLER, "failed to map device memory to virtual memory");
        pDeviceExtension->RegistersMapped = FALSE;
        return STATUS_NONE_MAPPED;
    }

    pDeviceExtension->ControllerAddressSpace = pUartHardwareConfiguration->AddressSpace;
    pDeviceExtension->ControllerMemorySpan = pUartHardwareConfiguration->MemoryLength;

    pDeviceExtension->InterruptVector = pUartHardwareConfiguration->InterruptVector;
    pDeviceExtension->InterruptLevel = (KIRQL) pUartHardwareConfiguration->InterruptLevel;
    pDeviceExtension->InterruptAffinity = pUartHardwareConfiguration->InterruptAffinity;

    WRITE_UART_ENABLE(pDeviceExtension, TRUE);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "enabled uart device");

    // FIFO is always enabled on BCM2836 so we only need to reset it

    UCHAR fifoControlRegister = (UCHAR)
    (
        AUX_MU_IIR_REGISTER_W_RESET_RECEIVE_FIFO |
        AUX_MU_IIR_REGISTER_W_RESET_TRANSMIT_FIFO
    );

    WRITE_FIFO_CONTROL(pDeviceExtension, fifoControlRegister);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "wrote FIFO control UCHAR 0x%x", (unsigned int) fifoControlRegister);

    pDeviceExtension->LineStatus = READ_LINE_STATUS(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "read line status UCHAR 0x%x", (unsigned int) pDeviceExtension->LineStatus);

    pDeviceExtension->ModemStatus = READ_MODEM_STATUS(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "read modem status UCHAR 0x%x", (unsigned int) pDeviceExtension->ModemStatus);

    pDeviceExtension->ModemControl = 0;
    pDeviceExtension->LineControl = 0;
    pDeviceExtension->DivisorLatch = 0;
    pDeviceExtension->ClockRate = 0;
    pDeviceExtension->DeviceActive = FALSE;

    pDeviceExtension->TxFifoSize = SERIAL_TX_FIFO_SIZE_DEFAULT;
    pDeviceExtension->RxFifoSize = SERIAL_RX_FIFO_SIZE_DEFAULT;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID UninitializeUartController(_In_ WDFDEVICE device)
{
    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);
    WRITE_UART_ENABLE(pDeviceExtension, FALSE);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "disabled uart device");
}

NTSTATUS UartInterruptEvtInterruptEnable(_In_ WDFINTERRUPT interrupt, _In_ WDFDEVICE associatedDevice)
{
    UNREFERENCED_PARAMETER(interrupt);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(associatedDevice);

    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}
