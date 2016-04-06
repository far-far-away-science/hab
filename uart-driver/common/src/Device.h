#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

NTSTATUS UartDeviceCreate(_In_ PWDFDEVICE_INIT deviceInit);

EVT_WDF_DEVICE_PREPARE_HARDWARE UartDeviceEvtPrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE UartDeviceEvtReleaseHardware;
