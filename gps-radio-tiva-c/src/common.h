#pragma once

#include <stdint.h>

#define CPU_SPEED 16000000

#define CHANNEL_VENUS_GPS      0
#define CHANNEL_COPERNICUS_GPS 1
#define CHANNEL_TELEMETRY_MCU  2
#ifdef DEBUG
#define CHANNEL_DEBUG          3
#endif

#define RADIO_MCU_MESSAGE_SENDING_INTERVAL_SECONDS 30

enum GpsDataSource
{
    VENUS_GPS_ID      = '1',
    COPERNICUS_GPS_ID = '2',
};
