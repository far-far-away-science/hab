#pragma once

#include <ntddk.h>
#include <wdf.h>

#include "..\Trace.h"

EVT_WDF_INTERRUPT_ISR UartInterruptISR;
EVT_WDF_INTERRUPT_DPC UartInterruptTxRxDPCForISR;
EVT_WDF_INTERRUPT_DISABLE UartInterruptEvtInterruptDisable;
