#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

// don't forget to call this, if you don't behaviour is undefined
void initializeVenusGps(void);

// pResultBuffer must be properly allocated buffer
// if false is returned it means there are no messages available, pResultBuffer won't be touched in this case
bool readVenusGpsMessage(struct Message* pResultBuffer);
