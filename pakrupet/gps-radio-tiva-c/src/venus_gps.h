#pragma once

#include <stdint.h>
#include <stdbool.h>

void initializeVenusGps(void);

struct VenusGpsMessage
{
    uint8_t message[10];
};

/*
 * returns false if no message is available or buffer is nullptr
 */
bool readLatestMessageFromVenusGps(struct VenusGpsMessage* pMessage);
