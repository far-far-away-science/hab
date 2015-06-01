#pragma once

#include "uart.h"
#include "nmea_messages.h"

void createAprsMessage(const struct GpsData* pGpsData, struct Message* pResult);
