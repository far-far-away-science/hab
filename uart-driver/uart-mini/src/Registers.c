#include "Registers.h"

#include "..\..\Trace.h"

#include "Registers.tmh"

BOOLEAN CanReadCharacter(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return FALSE;
}

BOOLEAN IsFifoDataLoss(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return FALSE;
}

VOID UART_DEVICE_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ BOOLEAN enable)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    UNREFERENCED_PARAMETER(enable);
}

USHORT READ_SERIAL_TX_FIFO_SIZE()
{
    return 16;
}

USHORT READ_SERIAL_RX_FIFO_SIZE()
{
    return 16;
}

VOID FIFO_CONTROL_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

UCHAR LINE_STATUS_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return 0;
}

UCHAR MODEM_STATUS_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return 0;
}

VOID LINE_CONTROL_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineControlValue)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    UNREFERENCED_PARAMETER(lineControlValue);
}

VOID MODEM_CONTROL_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR modemControlValue)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    UNREFERENCED_PARAMETER(modemControlValue);
}

VOID MODEM_CONTROL_DISABLE_REQUEST_TO_SEND(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

VOID DIVISOR_LATCH_WRITE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT divisorValue)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    UNREFERENCED_PARAMETER(divisorValue);
}

UCHAR RECEIVE_BUFFER_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return 0;
}

VOID INTERRUPT_DISABLE_ALL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

VOID INTERRUPT_ENABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

VOID INTERRUPT_DISABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

VOID INTERRUPT_ENABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

VOID INTERRUPT_DISABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
}

UCHAR INTERRUPT_READ_MASKED_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    return 0;
}
