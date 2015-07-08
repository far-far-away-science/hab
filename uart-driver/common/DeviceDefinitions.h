#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include "src\CommonDefinitions.h"

NTSTATUS InitializeUartController(_In_ WDFDEVICE device, _In_ const UART_HARDWARE_CONFIGURATION* pUartHardwareConfiguration);
VOID UninitializeUartController(_In_ WDFDEVICE device);

BOOLEAN GetCanReadFromLineStatus(_In_ WDFDEVICE device);
VOID LogLineStatusErrors(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineStatusRegister);

UCHAR READ_RECEIVE_BUFFER(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
VOID DISABLE_ALL_INTERRUPTS(_In_ PUART_DEVICE_EXTENSION pDeviceExtension);
VOID WRITE_DIVISOR_LATCH(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ USHORT divisorValue);
VOID WRITE_LINE_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR lineControlValue);
VOID WRITE_MODEM_CONTROL(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR modemControlValue);
VOID WRITE_INTERRUPT_ENABLE(_In_ PUART_DEVICE_EXTENSION pDeviceExtension, _In_ UCHAR interruptValue);
