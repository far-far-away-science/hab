#pragma once

#define INITGUID

#include <ntddk.h>
#include <wdf.h>

#include "..\Trace.h"

EXTERN_C_START

    DRIVER_INITIALIZE DriverEntry;

EXTERN_C_END

EVT_WDF_DRIVER_DEVICE_ADD UartDriverEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP UartDriverEvtDriverContextCleanup;
