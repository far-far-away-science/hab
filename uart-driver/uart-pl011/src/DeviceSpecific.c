#include "pl011.h"
#include "..\..\Trace.h"

#include "DeviceSpecific.tmh"

USHORT READ_SERIAL_TX_FIFO_SIZE()
{
    return 16;
}

USHORT READ_SERIAL_RX_FIFO_SIZE()
{
    return 16;
}

VOID UART_DEVICE_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ BOOLEAN enable)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    UNREFERENCED_PARAMETER(enable);
}

VOID INITIALIZE_AND_BACKUP_REGISTERS_IN_MEMORY(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "initializing and caching registers");
    pDeviceExtension->ControlRegister = REGISTER_CR_READ(pDeviceExtension);
    pDeviceExtension->LineControlRegister = REGISTER_LCRH_READ(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "finished initializing and caching registers");
}

VOID RESTORE_REGISTERS_FROM_MEMORY(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "writing cached registers to device");
    REGISTER_CR_WRITE(pDeviceExtension, pDeviceExtension->ControlRegister);
    REGISTER_LCRH_WRITE(pDeviceExtension, pDeviceExtension->LineControlRegister);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "finished writing cached registers to device");
}

VOID FIFO_CONTROL_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabling FIFO");
    DisableUart(pDeviceExtension);
    pDeviceExtension->LineControlRegister |= REGISTER_LCRH_FEN;
    REGISTER_LCRH_WRITE(pDeviceExtension, pDeviceExtension->LineControlRegister);
    EnableUart(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabled FIFO");
}

VOID INTERRUPT_DISABLE_ALL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 3 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 3 %!FUNC! Exit");
}

VOID INTERRUPT_ENABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 4 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 4 %!FUNC! Exit");
}

VOID INTERRUPT_DISABLE_LINE_STATUS_CHANGE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 5 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 5 %!FUNC! Exit");
}

VOID INTERRUPT_ENABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 6 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 6 %!FUNC! Exit");
}

BOOLEAN INTERRUPT_DISABLE_RECEIVE_DATA_AVAILABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 7 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 7 %!FUNC! Exit");
    return TRUE;
}

VOID DISABLE_REQUEST_TO_SEND(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "Disabling RTS");
    pDeviceExtension->ControlRegister &= ~REGISTER_CR_RTS;
    REGISTER_CR_WRITE(pDeviceExtension, pDeviceExtension->ControlRegister);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "Disabled RTS");
}

BOOLEAN IS_FIFO_DATA_LOSS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 9 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 9 %!FUNC! Exit");
    return FALSE;
}

BOOLEAN CAN_RECEIVE_CHARACTER(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 10 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 10 %!FUNC! Exit");
    return FALSE;
}

UCHAR RECEIVE_BUFFER_READ(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    UNREFERENCED_PARAMETER(pDeviceExtension);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 11 %!FUNC! Entry");
    // TODO
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "--- 11 %!FUNC! Exit");
    return 0;
}
