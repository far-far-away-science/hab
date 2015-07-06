#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\CommonDefinitions.h"

#include "..\..\Trace.h"

NTSTATUS InitializeUartController(_In_ WDFDEVICE device, _In_ const UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration);

EVT_WDF_DEVICE_D0_ENTRY PowerEvtD0Entry;

EVT_WDF_INTERRUPT_ENABLE UartInterruptEvtInterruptEnable;
