#include "tiva_c.h"
#include "signals.h"
#include "venus_gps.h"

#include <string.h>

int main()
{
    initializeTivaC();
    initializeSignals();
    initializeVenusGps();

    signalSuccess();

    struct VenusGpsMessage venusGpsMessage;

    while (true)
    {
        if (readVenusGpsMessage(&venusGpsMessage) && venusGpsMessage.size > 6)
        {
            if (memcmp(venusGpsMessage.message, "$GP", 3) == 0)
            {
                if (memcmp(venusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    // TODO send simplified position information to radio
                    // TODO send full position information to telemetry board
                }
                else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    // TODO send full ground speed information to telemetry board
                }
            }
        }
    }
}
