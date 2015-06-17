#include "uart.h"
#include "timer.h"
#include "tiva_c.h"
#include "common.h"
#include "signals.h"
#include "aprs_board.h"

#include <string.h>

#include <driverlib/rom.h>
#include <driverlib/systick.h>

int main()
{
    initializeTivaC();
    initializeAprs();
    initializeTimer();
    initializeUart();
    initializeSignals();

    bool r = true;

    r &= initializeUartChannel(CHANNEL_VENUS_GPS, UART_1, 9600, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_COPERNICUS_GPS, UART_2, 4800, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_TELEMETRY_MCU, UART_3, 115200, CPU_SPEED, UART_FLAGS_SEND);
#ifdef DEBUG
    r &= initializeUartChannel(CHANNEL_DEBUG, UART_0, 115200, CPU_SPEED, UART_FLAGS_SEND);
#endif

    if (r)
    {
        signalSuccess();
    }
    else
    {
        signalError();
    }

    uint32_t startTime = getSecondsSinceStart();

    struct GpsData venusGpsData = { 0 };
    struct GpsData copernicusGpsData = { 0 };

    struct Message venusGpsMessage = { 0 };
    struct Message copernicusGpsMessage = { 0 };

    bool shouldSendVenusDataToAprs = true;
    
    while (true)
    {
        if (readMessage(CHANNEL_VENUS_GPS, &venusGpsMessage) && venusGpsMessage.size > 6)
        {
            if (memcmp(venusGpsMessage.message, "$GP", 3) == 0)
            {
                if (memcmp(venusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    parseGpggaMessageIfValid(&venusGpsMessage, &venusGpsData);
                    venusGpsMessage.message[0] = VENUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &venusGpsMessage);
                }
                else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    parseGpvtgMessageIfValid(&venusGpsMessage, &venusGpsData);
                    venusGpsMessage.message[0] = VENUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &venusGpsMessage);
                }
            }
        }

        if (readMessage(CHANNEL_COPERNICUS_GPS, &copernicusGpsMessage) && copernicusGpsMessage.size > 6)
        {
            if (memcmp(copernicusGpsMessage.message, "$GP", 3) == 0)
            {
                if (memcmp(copernicusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    parseGpggaMessageIfValid(&copernicusGpsMessage, &copernicusGpsData);
                    copernicusGpsMessage.message[0] = COPERNICUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &copernicusGpsMessage);
                }
                else if (memcmp(copernicusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    parseGpvtgMessageIfValid(&copernicusGpsMessage, &copernicusGpsData);
                    copernicusGpsMessage.message[0] = COPERNICUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &copernicusGpsMessage);
                }
            }
        }

        uint32_t currentTime = getSecondsSinceStart();

        if (currentTime - startTime >= 5 /* RADIO_MCU_MESSAGE_SENDING_INTERVAL_SECONDS */)
        {
            if (shouldSendVenusDataToAprs && venusGpsData.isValid)
            {
                sendAprsMessage(&venusGpsData);
            }
            else
            {
                // higher chance that copernicus will work more reliably
                // so we will use it as a default fallback
                sendAprsMessage(&copernicusGpsData);
            }
            shouldSendVenusDataToAprs = !shouldSendVenusDataToAprs;
            startTime = currentTime;
        }

        // TODO if 60 seconds expired write stats to EPPROM
    }
}