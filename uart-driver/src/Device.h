#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\Trace.h"

typedef struct _UART_DEVICE_EXTENSION
{
    WDFDEVICE WdfDevice;
    PDEVICE_OBJECT DeviceObject;
    PDRIVER_OBJECT DriverObject;
} UART_DEVICE_EXTENSION, *PUART_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(UART_DEVICE_EXTENSION, UartGetDeviceExtension);

NTSTATUS UartDeviceCreate(_In_ PWDFDEVICE_INIT deviceInit);

EVT_WDF_DEVICE_PREPARE_HARDWARE UartDeviceEvtPrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE UartDeviceEvtReleaseHardware;
EVT_WDF_DEVICE_D0_ENTRY UartDeviceEvtD0Entry;
EVT_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED UartDeviceEvtD0EntryPostInterruptsEnabled;
EVT_WDF_DEVICE_D0_EXIT UartDeviceEvtD0Exit;
EVT_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED UartDeviceEvtD0ExitPreInterruptsDisabled;
EVT_WDF_OBJECT_CONTEXT_DESTROY UartDeviceEvtDestroy;
