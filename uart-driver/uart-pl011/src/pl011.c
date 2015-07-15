#include "pl011.h"

#include "..\..\Trace.h"

#include "pl011.tmh"

FORCEINLINE VOID WriteRegisterUShort(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset, _In_ USHORT value)
{
    WRITE_REGISTER_USHORT((PUSHORT) (((PUCHAR) baseAddress) + offset), value);
    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_RECEIVE,
                "wrote register USHORT (0x%p + 0x%lx) = 0x%04x",
                (void*) baseAddress,
                (long int) offset,
                (unsigned int) value);
}

USHORT ReadRegisterUShort(_In_reads_(_Inexpressible_(offset)) REGBASE baseAddress, _In_ ULONG offset)
{
    const USHORT result = READ_REGISTER_USHORT((PUSHORT) (((PUCHAR) baseAddress) + offset));
    TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_RECEIVE,
                "read register UCHAR (0x%p + 0x%lx) = 0x%04x",
                (void*) baseAddress,
                (long int) offset,
                (unsigned int) result);
    return result;
}

VOID EnableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    if ((pDeviceExtension->ControlRegister & REGISTER_CR_UARTEN) == 0)
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabling UART");
        pDeviceExtension->ControlRegister |= REGISTER_CR_UARTEN;
        REGISTER_CR_WRITE(pDeviceExtension, pDeviceExtension->ControlRegister);
        REGISTER_LCRH_WRITE(pDeviceExtension, pDeviceExtension->LineControlRegister); // restore line control as we changed it in disable
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "enabled UART");
    }
    else
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "UART already enabled");
    }
}

VOID DisableUart(_In_ PUART_DEVICE_EXTENSION pDeviceExtension)
{
    if (pDeviceExtension->ControlRegister & REGISTER_CR_UARTEN)
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "disabling UART");
        pDeviceExtension->ControlRegister &= ~REGISTER_CR_UARTEN;
        REGISTER_CR_WRITE(pDeviceExtension, pDeviceExtension->ControlRegister); // disable UART
        REGISTER_LCRH_WRITE(pDeviceExtension, pDeviceExtension->LineControlRegister & ~REGISTER_LCRH_FEN); // flush FIFO
        while (REGISTER_FR_READ(pDeviceExtension) & REGISTER_FR_BUSY)
        {
            // TODO do something here so as not to waste CPU cycles
            // TODO need to figure out how to sleep in kernel drivers for a short time
        }
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "disabled UART");
    }
    else
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_UART_PL011, "UART already disabled");
    }
}
