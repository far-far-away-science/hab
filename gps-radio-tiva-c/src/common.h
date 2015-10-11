#pragma once

#include <stdint.h>

#define CPU_SPEED 16000000

#define CHANNEL_VENUS_GPS      0
#define CHANNEL_COPERNICUS_GPS 1
#ifdef DUMP_DATA_TO_UART0
#define CHANNEL_OUTPUT         2
#endif

// Send messages every 30 seconds
#define RADIO_MCU_MESSAGE_SENDING_INTERVAL 30
// Dithering of up to 3 seconds
#define RADIO_MCU_MESSAGE_DITHER 3
// Issue #5: Send signals more often when closer to the ground
// If altitude is less than 3000 m ASL, frequency increases to 15 seconds
#define RADIO_MCU_LOW_ALTITUDE 3000
#define RADIO_MCU_MESSAGE_FAST_INTERVAL 15

typedef enum GpsDataSource_t
{
    VENUS_GPS_ID      = 1,
    COPERNICUS_GPS_ID = 2,
} GpsDataSource;
