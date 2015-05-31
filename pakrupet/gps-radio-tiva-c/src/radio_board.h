#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "uart.h"

bool createRadioBoardMessageFromGpggaMessage(uint8_t gpsSourceCode,
                                             const struct Message* pGpsGpggaMessage,
                                             struct Message* pRadioBoardGpsMessage);
