#include "uart.h"
#include "timer.h"
#include "tiva_c.h"
#include "common.h"
#include "signals.h"
#include "telemetry.h"
#include "aprs_board.h"
#include "i2c.h"
#include "eeprom.h"

#include <stdio.h>
#include <string.h>

#include <driverlib/rom.h>
#include <driverlib/systick.h>

// Reduce stack usage by main() and get a "free" zero initialization!
static GpsData venusGpsData;
static GpsData copernicusGpsData;
static Message venusGpsMessage;
static Message copernicusGpsMessage;
static Telemetry telemetry;

#ifdef DUMP_DATA_TO_UART0
    static Message telemetryMessage;
#endif

#ifdef EEPROM_ENABLED            
    // EEPROM recording buffer
    static uint32_t eepromBuffer;
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
// 0x10 DLE
// 0x03 ETX
static const Message airMode = {
    42, {
        0x10, 0xBB, 0x03, 0x00, 0x00, 0x03, 0x00, 0xC2, 0xB8, 0xB2, 0x3D, 0x9A, 0x99, 0x19,
        0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x03
    }
};

// Initialize the board
static inline uint32_t init(void)
{
    bool r = true;

    // Call initialize methods in individual subsystem files
    initializeTivaC();
    initializeSignals();
    initializeAprs();
    initializeTimer();
    initializeUart();
    initializeTelemetry();

#ifdef EEPROM_ENABLED            
    // If button 1 is held down during power up/reset, then EEPROM recording will be activated
    uint32_t record = isUserButton1() ? 0U : 0xFFFFFFFFU;
    initializeEEPROM(record == 0U);
    eepromBuffer = 0U;
#endif
    initializeI2C();

    // Configure UART channels
    r &= initializeUartChannel(CHANNEL_VENUS_GPS, UART_1, 9600, CPU_SPEED, UART_FLAGS_RECEIVE);
    r &= initializeUartChannel(CHANNEL_COPERNICUS_GPS, UART_2, 4800, CPU_SPEED, UART_FLAGS_RECEIVE);
#ifdef DUMP_DATA_TO_UART0
    r &= initializeUartChannel(CHANNEL_OUTPUT, UART_0, 115200, CPU_SPEED, UART_FLAGS_SEND);
#endif
    r &= writeMessage(CHANNEL_COPERNICUS_GPS, &airMode);
    if (r)
    {
        signalSuccess();
    }
    else
    {
        signalError();
    }
    return record;
}

// Reads and updates GPS module data
static void updateGPS(uint32_t channel, Message *messageIn, GpsData *dataOut)
{
    // If a message is available
    if (readMessage(channel, messageIn) && messageIn->size > 6)
    {
#ifdef DUMP_DATA_TO_UART0
        // Debugging usage only
        if (channel == CHANNEL_VENUS_GPS)
        {
            writeString(CHANNEL_OUTPUT, "vens - ");
        }
        else
        {
            writeString(CHANNEL_OUTPUT, "copr - ");
        }
        writeMessage(CHANNEL_OUTPUT, messageIn);
#endif
        if (memcmp(messageIn->message, "$GP", 3) == 0)
        {
            bool update = false;
            if (memcmp(messageIn->message + 3, "GGA", 3) == 0)
            {
                // Global Positioning System fix
                parseGpggaMessageIfValid(messageIn, dataOut);
                update = true;
            }
            else if (memcmp(venusGpsMessage.message + 3, "VTG", 3) == 0)
            {
                // Track made good and ground speed
                parseGpvtgMessageIfValid(messageIn, dataOut);
                update = true;
            }
            if (update)
            {
                // The GPS can be set up to disable all the other messages in theory
                // Conveniently enough, the channels match the I2C indices
                submitI2CData(channel, dataOut);
            }
        }
    }
}

// Sends an APRS message
static inline uint32_t sendAPRSMessage(uint32_t now, bool *sendVenusData, uint32_t *ditherCount)
{
    uint32_t dither, alt = 0U;
    const bool shouldSendVenusDataToAprs = *sendVenusData;
    
    // Fetch telemetry data
    getTelemetry(&telemetry);
#ifdef DUMP_DATA_TO_UART0
    // Debugging only
    telemetryMessage.size = sprintf((char*) telemetryMessage.message, "tele - temp=%u, vcc=%u\r\n", telemetry.cpuTemperature, telemetry.voltage);
    writeMessage(CHANNEL_OUTPUT, &telemetryMessage);
#endif
    // Update I2C registers
    submitI2CTelemetry(&telemetry);
    
    if (shouldSendVenusDataToAprs && venusGpsData.latitude.isValid && venusGpsData.longitude.isValid)
    {
        // venus data
        alt = venusGpsData.altitudeMslMeters;
        sendAprsMessage(VENUS_GPS_ID, &venusGpsData, &telemetry);
    }
    else
    {
        // higher chance that copernicus will work more reliably
        // so we will use it as a default fallback
        alt = copernicusGpsData.altitudeMslMeters;
        sendAprsMessage(COPERNICUS_GPS_ID, &copernicusGpsData, &telemetry);
    }
    *sendVenusData = !shouldSendVenusDataToAprs;
    
#if defined(RADIO_MCU_MESSAGE_DITHER) && (RADIO_MCU_MESSAGE_DITHER > 0)
    {
        // Perform dithering, correctly this time!
        const uint32_t ditherCountValue = *ditherCount;
        dither = (ditherCountValue % RADIO_MCU_MESSAGE_DITHER);
        *ditherCount = ditherCountValue + 1;
    }
#else
    dither = 0U;
#endif
    // Issue #5: Send messages more frequently near the ground
    if (alt > 0U && alt < RADIO_MCU_LOW_ALTITUDE)
    {
        dither += RADIO_MCU_MESSAGE_FAST_INTERVAL;
    }
    else
    {
        dither += RADIO_MCU_MESSAGE_SENDING_INTERVAL;
    }
    now += dither;
    // Next radio send time
    return now;
}

// Writes data to the EEPROM
#ifdef EEPROM_ENABLED
static inline uint32_t writeEEPROM(uint32_t record)
{
    // Every 30 seconds, write stats to EEPROM
    if (record < 2048U)
    {
        // Compose 16-bit EEPROM word = [LSB] one byte temp, [MSB] one byte voltage
        // Voltage = (mV - 4990) / 20
        // Temperature = (raw reading - 1595) / 10
        uint32_t result = ((telemetry.cpuTemperature - 1595U) / 10U) & 0xFFU;
        result |= (((telemetry.voltage - 4990U) / 20U) & 0xFFU) << 8;
        if (record & 2U)
        {
            eepromBuffer |= (result << 16U);
            // Finish up the buffer, and write the word at the base address
            eepromWrite(record & 0x7FC, &eepromBuffer);
        }
        else
        {
            // Write first half of the buffer value
            eepromBuffer = result;
        }
        record += 2U;
    }
    return record;
}
#endif

int main()
{
    bool shouldSendVenusDataToAprs = true;
    uint32_t currentTime, nextRadioSendTime = 5U, ditherCount = 0U;
    // Initialize board
    uint32_t record = init();
    // Start the watchdog
    startWatchdog();
    while (true)
    {
        // GPS data update
        updateGPS(CHANNEL_VENUS_GPS, &venusGpsMessage, &venusGpsData);
        updateGPS(CHANNEL_COPERNICUS_GPS, &copernicusGpsMessage, &copernicusGpsData);
        
        currentTime = getSecondsSinceStart();
        // If user button 1 is down, send APRS message "now"
        if (isUserButton1())
        {
            nextRadioSendTime = currentTime + 1U;
        }

        if (currentTime >= nextRadioSendTime)
        {
            // Send message
            nextRadioSendTime = sendAPRSMessage(currentTime, &shouldSendVenusDataToAprs, &ditherCount);
            // EEPROM
#ifdef EEPROM_ENABLED
            record = writeEEPROM(record);
#endif
        }
        
        // Blink green light to let everyone know that we are still running
        if (currentTime & 1)
        {
            signalHeartbeatOff();
        }
        else
        {
            signalHeartbeatOn();
        }
        feedWatchdog();
        
        // Enter low power mode
        ROM_SysCtlSleep();
    }
}
