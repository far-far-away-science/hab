#include "Registers.h"

#include "..\..\Trace.h"

#include "Registers.tmh"

VOID LogLineStatusEvents(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineStatusRegister)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

    // only this error flag is supported by this UART chip

    if (lineStatusRegister & AUX_MU_LSR_REGISTER_R_OVERRUN_ERROR)
    {
        const long int newValue = InterlockedIncrement(&pDeviceExtension->ErrorCount.FifoOverrunError);
        // TODO LOG in ETW
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BCM_2836_CONTROLLER, "fifo overrun error (count = %li)", (long int) newValue);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
}

BOOLEAN GetCanReadFromLineStatus(_In_ WDFDEVICE device)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_REGISTERS, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    UCHAR lineStatusRegisterValue;
    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);
    pDeviceExtension->LineStatus = READ_LINE_STATUS(pDeviceExtension);
    lineStatusRegisterValue = pDeviceExtension->LineStatus;
    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    LogLineStatusEvents(pDeviceExtension, lineStatusRegisterValue);

    const BOOLEAN result = (lineStatusRegisterValue & AUX_MU_LSR_REGISTER_R_DATA_READY) != 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BCM_2836_CONTROLLER, "%!FUNC! Exit");

    return result;
}
