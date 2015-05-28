#pragma once

#include <stdint.h>
#include <stdbool.h>

#define VENUS_GPS_MESSAGE_MAX_LEN 128

struct VenusGpsMessage
{
    uint8_t size;
    // to make sure there is no overflow check last 2 characters are 0x0D,0x0A.
    uint8_t message[VENUS_GPS_MESSAGE_MAX_LEN + 1];
};

// don't forget to call this, if you don't behaviour is undefined
void initializeVenusGps(void);

// pResultBuffer must be properly allocated buffer
// if false is returned it means there are no messages available, pResultBuffer won't be touched in this case
bool readVenusGpsMessage(struct VenusGpsMessage* pResultBuffer);
