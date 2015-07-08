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
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_UART_MINI_CONTROLLER, "fifo overrun error (count = %li)", (long int) newValue);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");
}

BOOLEAN GetCanReadFromLineStatus(_In_ WDFDEVICE device)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Entry");

    PUART_DEVICE_EXTENSION pDeviceExtension = GetUartDeviceExtension(device);

    UCHAR lineStatusRegisterValue;
    WdfInterruptAcquireLock(pDeviceExtension->WdfInterrupt);
    pDeviceExtension->LineStatus = READ_LINE_STATUS(pDeviceExtension);
    lineStatusRegisterValue = pDeviceExtension->LineStatus;
    WdfInterruptReleaseLock(pDeviceExtension->WdfInterrupt);

    LogLineStatusEvents(pDeviceExtension, lineStatusRegisterValue);

    const BOOLEAN result = (lineStatusRegisterValue & AUX_MU_LSR_REGISTER_R_DATA_READY) != 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_MINI_CONTROLLER, "%!FUNC! Exit");

    return result;
}

VOID WRITE_UART_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ BOOLEAN uartEnable)
{
    REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    UCHAR deviceEnabledFlags = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_ENABLES);
    if (uartEnable)
    {
        deviceEnabledFlags |= AUX_ENABLES_ENABLE_UART;
    }
    else
    {
        deviceEnabledFlags &= ~AUX_ENABLES_ENABLE_UART;
    }
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_ENABLES, deviceEnabledFlags);
}

VOID WRITE_INTERRUPT_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR interruptValue)
{
    REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IER_REGISTER, interruptValue);
}

VOID DISABLE_ALL_INTERRUPTS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    WRITE_INTERRUPT_ENABLE(pDeviceExtension, 0);
}

VOID WRITE_FIFO_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR fifoControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IIR_REGISTER, fifoControlValue);
}

UCHAR READ_LINE_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LSR_REGISTER);
}

VOID WRITE_LINE_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControlValue);
}

UCHAR READ_MODEM_STATUS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_MSR_REGISTER);
}

VOID WRITE_MODEM_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR modemControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_MCR_REGISTER, modemControlValue);
}

VOID WRITE_DIVISOR_LATCH(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT divisorValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    const UCHAR lineControl = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER);
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, (lineControl | AUX_MU_LCR_REGISTER_W_DLAB));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_LSB_REGISTER, (divisorValue & 0xff));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_MSB_REGISTER, ((divisorValue & 0xff00) >> 8));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControl);
}

UCHAR READ_RECEIVE_BUFFER(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_IO_REGISTER);
}

UCHAR READ_INTERRUPT_ID_REG(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_IIR_REGISTER);
}
