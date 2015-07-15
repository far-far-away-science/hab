#include "pl011.h"

#include "..\..\Trace.h"

#include "pl011.tmh"

VOID EnableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabling UART");
    pDeviceExtension->ModemControl |= REGISTER_CR_UARTEN;
    REGISTER_CR_WRITE(pDeviceExtension, pDeviceExtension->ModemControl);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabled UART");
}

VOID DisableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "disabling UART");
    pDeviceExtension->ModemControl = REGISTER_CR_READ(pDeviceExtension);
    if (pDeviceExtension->ModemControl & REGISTER_CR_UARTEN)
    {
        // TODO
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "disabling UART isn't implemented");
        // TODO
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "disabled UART");
}
