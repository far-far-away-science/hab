#pragma once

#define INITGUID

#include <ntddk.h>
#include <wdf.h>
#include <usb.h>
#include <usbdlib.h>
#include <wdfusb.h>

#include "trace.h"

EXTERN_C_START

    DRIVER_INITIALIZE DriverEntry;
    EVT_WDF_DRIVER_DEVICE_ADD UartDriverEvtDeviceAdd;
    EVT_WDF_OBJECT_CONTEXT_CLEANUP UartDriverEvtDriverContextCleanup;

EXTERN_C_END
