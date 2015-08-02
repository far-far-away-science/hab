#include "uart.h"
#include "timer.h"
#include "tiva_c.h"
#include "common.h"
#include "signals.h"
#include "telemetry.h"
#include "aprs_board.h"
#include "i2c.h"

#include <string.h>

#include <driverlib/rom.h>
#include <driverlib/systick.h>

// Reduce stack usage by main() and get a "free" zero initialization!
static GpsData venusGpsData;
static GpsData copernicusGpsData;
static Message venusGpsMessage;
static Message copernicusGpsMessage;
static Telemetry telemetry;

int main()
{
    initializeTivaC();
    initializeAprs();
    initializeTimer();
    initializeUart();
    initializeSignals();
    initializeI2C();
    initializeTelemetry();

    bool r = true;

    r &= initializeUartChannel(CHANNEL_VENUS_GPS, UART_1, 9600, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_COPERNICUS_GPS, UART_2, 4800, CPU_SPEED, UART_FLAGS_RECEIVE);
#ifdef DEBUG
    r &= initializeUartChannel(CHANNEL_DEBUG, UART_0, 115200, CPU_SPEED, UART_FLAGS_SEND);
#endif
    
    // AIR mode for Copernicus:
    // 0x10 DLE
    // 0xBB Set GPS Processing Options
    // 0x03 Query Mode
    // 0x00 Auto 2D/3D
    // 0x00 Reserved
    // 0x03 Dynamics Mode AIR
    // 0x00 Reserved
    //  0x3db2b8c2 = 5 degrees in radians little endian (0.087266), Copernicus is big endian
    // 0xC2 Lowest satellite elevation angle for fix
    // 0xB8
    // 0xB2
    // 0x3D
    //  0x3f19999a = 0.6 little endian
    // 0x9A Minimum signal level for fix
    // 0x99
    // 0x19
    // 0x3F
    // 0x00 * 27 (4 + 4 + 1 + 18) Reserved

    if (r)
    {
        signalSuccess();
    }
    else
    {
        signalError();
    }

    bool shouldSendVenusDataToAprs = true;

    uint32_t lastRadioSentTime = getSecondsSinceStart();
    
    while (true)
    {
        if (readMessage(CHANNEL_VENUS_GPS, &venusGpsMessage) && venusGpsMessage.size > 6)
        {
            if (memcmp(venusGpsMessage.message, "$GP", 3) == 0)
            {
                bool update = false;
                if (memcmp(venusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    parseGpggaMessageIfValid(&venusGpsMessage, &venusGpsData);
                    update = true;
                }
                else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    parseGpvtgMessageIfValid(&venusGpsMessage, &venusGpsData);
                    update = true;
                }
                if (update)
                {
                    // The Venus can be set up to disable all the other messages in theory
                    submitI2CData(0, &venusGpsData);
                }
            }
        }

        if (readMessage(CHANNEL_COPERNICUS_GPS, &copernicusGpsMessage) && copernicusGpsMessage.size > 6)
        {
            if (memcmp(copernicusGpsMessage.message, "$GP", 3) == 0)
            {
                bool update = false;
                if (memcmp(copernicusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    parseGpggaMessageIfValid(&copernicusGpsMessage, &copernicusGpsData);
                    update = true;
                }
                else if (memcmp(copernicusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    parseGpvtgMessageIfValid(&copernicusGpsMessage, &copernicusGpsData);
                    update = true;
                }
                if (update)
                {
                    submitI2CData(1, &copernicusGpsData);
                }
            }
        }

        uint32_t currentTime = getSecondsSinceStart();

        if (currentTime - lastRadioSentTime >= RADIO_MCU_MESSAGE_SENDING_INTERVAL_SECONDS)
        {
            getTelemetry(&telemetry);
            submitI2CTelemetry(&telemetry);
            
            if (shouldSendVenusDataToAprs && venusGpsData.isValid)
            {
                sendAprsMessage(&venusGpsData, &telemetry);
            }
            else
            {
                // higher chance that copernicus will work more reliably
                // so we will use it as a default fallback
                sendAprsMessage(&copernicusGpsData, &telemetry);
            }

            shouldSendVenusDataToAprs = !shouldSendVenusDataToAprs;
            lastRadioSentTime = currentTime;
        }

        // TODO if 60 seconds expired write stats to EPPROM

        // Enter low power mode
        ROM_SysCtlSleep();
    }
}
