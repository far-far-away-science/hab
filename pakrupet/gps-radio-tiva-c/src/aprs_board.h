#pragma once

#include "nmea_messages.h"

void initializeAprs(void);
void sendAprsMessage(const struct GpsData* pGpsData);
