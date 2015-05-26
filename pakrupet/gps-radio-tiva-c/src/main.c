#include "tiva_c.h"
#include "signals.h"
#include "venus_gps.h"

int main()
{
    initializeTivaC();
    initializeSignals();
    initializeVenusGps();

    signalSuccess();

    while (true) {}
}
