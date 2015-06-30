#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "..\Trace.h"

NTSTATUS UartInitContext(_In_ WDFDEVICE device);

EVT_SERCX2_CONTROL UartEvtSerCx2Control;
EVT_SERCX2_PURGE_FIFOS UartEvtSerCx2PurgeFifos;
EVT_SERCX2_APPLY_CONFIG UartEvtSerCx2ApplyConfig;
EVT_SERCX2_SET_WAIT_MASK UartEvtSerCx2SetWaitMask;
