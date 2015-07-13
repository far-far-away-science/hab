#include "Registers.h"

#include "..\..\Trace.h"

#include "Registers.tmh"

VOID LogLineStatusEvents(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineStatusRegister)
{
    if (lineStatusRegister & AUX_MU_LSR_REGISTER_R_OVERRUN_ERROR)
    {
        const long int newValue = InterlockedIncrement(&pDeviceExtension->ErrorCount.FifoOverrunError);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_UART_MINI, "fifo overrun error (count = %li)", (long int) newValue);
    }
}

UCHAR LINE_STATUS_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->LineStatus = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LSR_REGISTER);
    LogLineStatusEvents(pDeviceExtension, pDeviceExtension->LineStatus);
    return pDeviceExtension->LineStatus;
}

BOOLEAN CanReadCharacter(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const UCHAR lineStatusRegisterValue = LINE_STATUS_READ(pDeviceExtension);
    return (lineStatusRegisterValue & AUX_MU_LSR_REGISTER_R_DATA_READY) != 0;
}

BOOLEAN IsFifoDataLoss(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const UCHAR lineStatusRegister = LINE_STATUS_READ(pDeviceExtension);
    return ((lineStatusRegister & AUX_MU_LSR_REGISTER_R_TRANSMITTER_EMPTY) == 0) ||
           ((lineStatusRegister & AUX_MU_LSR_REGISTER_R_TRANSMITTER_IDLE) == 0);
}

VOID UART_DEVICE_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ BOOLEAN enable)
{
    REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    UCHAR deviceEnabledFlags = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_ENABLES);
    if (enable)
    {
        deviceEnabledFlags |= AUX_ENABLES_UART;
    }
    else
    {
        deviceEnabledFlags &= ~AUX_ENABLES_UART;
    }

    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_ENABLES, deviceEnabledFlags);
}

USHORT READ_SERIAL_TX_FIFO_SIZE()
{
    return 8;
}

USHORT READ_SERIAL_RX_FIFO_SIZE()
{
    return 8;
}

FORCEINLINE VOID IIR_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR iirValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IIR_REGISTER, iirValue);
}

VOID FIFO_CONTROL_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    IIR_WRITE(pDeviceExtension, AUX_MU_IIR_REGISTER_W_RESET_RECEIVE_FIFO | AUX_MU_IIR_REGISTER_W_RESET_TRANSMIT_FIFO);
}

UCHAR MODEM_STATUS_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_MSR_REGISTER);
}

VOID LINE_CONTROL_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControlValue);
}

VOID MODEM_CONTROL_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR modemControlValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_MCR_REGISTER, modemControlValue);
}

VOID MODEM_CONTROL_DISABLE_REQUEST_TO_SEND(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    MODEM_CONTROL_WRITE(pDeviceExtension, pDeviceExtension->ModemControl & ~AUX_MU_MCR_REGISTER_W_REQUEST_TO_SEND);
}

VOID DIVISOR_LATCH_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT divisorValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    const UCHAR lineControl = pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER);
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, (lineControl | AUX_MU_LCR_REGISTER_W_DLAB));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_LSB_REGISTER, (divisorValue & 0xff));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_BAUD_RATE_MSB_REGISTER, ((divisorValue & 0xff00) >> 8));
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_LCR_REGISTER, lineControl);
}

UCHAR RECEIVE_BUFFER_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    return pDeviceExtension->UartReadDeviceUChar(baseAddress, AUX_MU_IO_REGISTER);
}

FORCEINLINE VOID IER_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR interruptValue)
{
    const REGBASE baseAddress = pDeviceExtension->ControllerAddress;
    pDeviceExtension->UartWriteDeviceUChar(baseAddress, AUX_MU_IER_REGISTER, interruptValue);
}

VOID INTERRUPT_DISABLE_ALL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    IER_WRITE(pDeviceExtension, 0);
}

VOID INTERRUPT_ENABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
    IER_WRITE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
}

VOID INTERRUPT_DISABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    const BOOLEAN interruptEnabled = (pDeviceExtension->InterruptEnableRegister & AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE) != 0;
    if (interruptEnabled)
    {
        pDeviceExtension->InterruptEnableRegister &= ~AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE;
        IER_WRITE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
    }
}

VOID INTERRUPT_ENABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    pDeviceExtension->InterruptEnableRegister |= AUX_MU_IER_REGISTER_W_INT_ENABLE_RECEIVE_DATA_AVAILABLE;
    IER_WRITE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
}

BOOLEAN INTERRUPT_DISABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    if ((pDeviceExtension->InterruptEnableRegister & AUX_MU_IER_REGISTER_W_INT_ENABLE_RECEIVE_DATA_AVAILABLE) != 0)
    {
        pDeviceExtension->InterruptEnableRegister &= ~AUX_MU_IER_REGISTER_W_INT_ENABLE_RECEIVE_DATA_AVAILABLE;
        IER_WRITE(pDeviceExtension, pDeviceExtension->InterruptEnableRegister);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
