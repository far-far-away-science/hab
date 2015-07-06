#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\Trace.h"

EVT_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED PowerEvtD0EntryPostInterruptsEnabled;
EVT_WDF_DEVICE_D0_EXIT PowerEvtD0Exit;
EVT_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED PowerEvtD0ExitPreInterruptsDisabled;

EVT_WDFDEVICE_WDM_POST_PO_FX_REGISTER_DEVICE PowerEvtDeviceWdmPostPoFxRegisterDevice;
EVT_WDFDEVICE_WDM_PRE_PO_FX_UNREGISTER_DEVICE PowerEvtDeviceWdmPrePoFxUnregisterDevice;
