#include "uart.h"
#include "tiva_c.h"
#include "common.h"
#include "signals.h"
#include "radio_board.h"

#include <string.h>

int main()
{
    initializeTivaC();
    initializeUart();
    initializeSignals();

    bool r = true;

    r &= initializeUartChannel(CHANNEL_VENUS_GPS, UART_1, 9600, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_COPERNICUS_GPS, UART_2, 9600, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_RADIO_MCU, UART_3, 1200, CPU_SPEED, UART_FLAGS_SEND);
    r &= initializeUartChannel(CHANNEL_TELEMETRY_MCU, UART_4, 115200, CPU_SPEED, UART_FLAGS_SEND);

    if (r)
    {
        signalSuccess();
    }
    else
    {
        signalError();
    }

    struct Message venusGpsMessage;
    struct Message radioBoardGpsMessage;

    // TODO need to aggreage and store last known good position
    // TODO if GPS stopped working we should send it with flags that we
    // TODO don't have the fix

    while (true)
    {
        if (readMessage(CHANNEL_VENUS_GPS, &venusGpsMessage) && venusGpsMessage.size > 6)
        {
            if (memcmp(venusGpsMessage.message, "$GP", 3) == 0)
            {
                if (memcmp(venusGpsMessage.message + 3, "GGA", 3) == 0)
                {
                    venusGpsMessage.message[0] = VENUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &venusGpsMessage);
                    if (createRadioBoardMessageFromGpggaMessage(VENUS_GPS_ID, &venusGpsMessage, &radioBoardGpsMessage))
                    {
                        // send 1 packet in 30 seconds
                        // when decending send more frequently
                        writeMessage(CHANNEL_RADIO_MCU, &radioBoardGpsMessage);
                    }
                }
                else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
                {
                    venusGpsMessage.message[0] = VENUS_GPS_ID;
                    writeMessage(CHANNEL_TELEMETRY_MCU, &venusGpsMessage);
                }
            }
        }
    }
}
