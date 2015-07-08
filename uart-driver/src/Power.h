#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\Trace.h"

EVT_WDFDEVICE_WDM_POST_PO_FX_REGISTER_DEVICE PowerEvtDeviceWdmPostPoFxRegisterDevice;
EVT_WDFDEVICE_WDM_PRE_PO_FX_UNREGISTER_DEVICE PowerEvtDeviceWdmPrePoFxUnregisterDevice;
