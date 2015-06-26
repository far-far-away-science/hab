#include "aprs_board.h"

#include <cmath>
#include <string.h>
#include <cstdio>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <driverlib/rom.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include "uart.h"
#include "timer.h"
#include "common.h"

// both counts below must be uneven
#define PREFIX_FLAGS_COUNT 1
#define SUFFIX_FLAGS_COUNT 1

#define APRS_MESSAGE_MAX_LEN   384
#define APRS_BITSTREAM_MAX_LEN 480 // bitstream will have extra bits in it so it must be larger than message buffer
                                   // in worst case we will insert extra 0 for every 5 bits

/*
 * PWM
 */

/*
 * those values are calculated in advance depending on MCU/etc
 */

#define PI 3.141592654f

#define PWM_STEP_SIZE 1.0f

#define PWM_PERIOD 650
#define PWM_MIN_PULSE_WIDTH 1
#define PWM_MAX_PULSE_WIDTH 647

#define F1200_COUNT 64.0f

/*
 * those values are calculated from prevous ones
 */

#define PULSES_PER_SYMBOL_COUNT F1200_COUNT

#define F2200_COUNT (1200.0f * F1200_COUNT / 2200.0f)

#define AMPLITUDE_SCALER ((float) (PWM_MAX_PULSE_WIDTH - PWM_MIN_PULSE_WIDTH) / 2.0f)
#define RECIPROCAL_AMPLITUDE_SCALER (1.0f / AMPLITUDE_SCALER)

#define AMPLITUDE_SHIFT ((float) (AMPLITUDE_SCALER + 1.0f))
#define AMPLITUDE_SHIFT_UINT (AMPLITUDE_SHIFT + 0.5f)

#define HALF_PERIOD_F1200 (F1200_COUNT / 2.0f)
#define HALF_PERIOD_F2200 (F2200_COUNT / 2.0f)

#define ANGULAR_FREQUENCY_F1200 (2.0f * PI / F1200_COUNT)
#define ANGULAR_FREQUENCY_F2200 (2200.0f * ANGULAR_FREQUENCY_F1200 / 1200.0f)

#define RECIPROCAL_ANGULAR_FREQUENCY_F1200 (1.0f / ANGULAR_FREQUENCY_F1200) 
#define RECIPROCAL_ANGULAR_FREQUENCY_F2200 (1.0f / ANGULAR_FREQUENCY_F2200) 

struct BitstreamPos
{
    uint16_t bitstreamCharIdx;
    uint8_t bitstreamCharBitIdx;
};

struct EncodingData
{
    uint8_t lastBit;
    uint8_t numberOfOnes;
    struct BitstreamPos bitstreamSize;
};

const struct Callsign CALLSIGN_SOURCE = 
{
    {"HABHAB"},
    0x61
};

const struct Callsign CALLSIGN_DESTINATION = 
{
    {"APRS  "},
    0xE0
};

bool g_sendingMessage = false;

struct BitstreamPos g_currentBitstreamPos = { 0 };
struct BitstreamPos g_currentBitstreamSize = { 0 };
uint8_t g_currentBitstream[APRS_BITSTREAM_MAX_LEN] = { 0 };

bool g_currentFrequencyIsF1200 = true;

float g_currentF1200Frame = 0;
float g_currentF2200Frame = 0;
uint8_t g_currentSymbolPulsesCount = 0;

void enableHx1(void);
void enablePwm(void);
void createAprsMessage(const struct GpsData* pGpsData);

void initializeAprs(void)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWM_PERIOD);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_MIN_PULSE_WIDTH);
}

void sendAprsMessage(const struct GpsData* pGpsData)
{
    if (g_sendingMessage)
    {
        return;
    }
    createAprsMessage(pGpsData);
    enableHx1();
    enablePwm();
}

void generateFcs(const uint8_t* pMessage, uint16_t messageSize, uint8_t* pFcs)
{
    uint16_t fcs = 0xFFFF;
    for (uint16_t i = 0; i < messageSize; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            const uint16_t shiftBit = fcs & 0x0001;
            fcs = fcs >> 1;
            if (shiftBit != ((pMessage[i] >> j) & 0x01))
            {
                fcs ^= 0x8408;
            }
        }
    }
    fcs ^= 0xFFFF;
    
    uint16_t fcsInverted = 0;
    for (int8_t i = 15; i >= 0; --i)
    {
        if (fcs & (1 << i))
        {
            fcsInverted |= 1 << (15 - i);
        }
    }
    pFcs[0] = fcsInverted & 0x00FF;
    pFcs[1] = (fcsInverted >> 8) & 0x00FF;
}

uint16_t generateMessage(const struct Callsign* pCallsignDestination,
                         const struct Callsign* pCallsignSource,
                         uint8_t* messageBuffer,
                         uint16_t maxMessageLen)
{
    if (maxMessageLen < 20)
    {
        return 0;
    }
    
    uint16_t messageSize = 0;
    
    for (uint8_t i = 0; i < PREFIX_FLAGS_COUNT; ++i)
    {
        messageBuffer[messageSize++] = '\x7E';
    }

    for (uint8_t i = 0; i < 6; ++i)
    {
        messageBuffer[messageSize++] = pCallsignDestination->callsign[i] << 1;
    }
    messageBuffer[messageSize++] = pCallsignDestination->ssid;
    for (uint8_t i = 0; i < 6; ++i)
    {
        messageBuffer[messageSize++] = pCallsignSource->callsign[i] << 1;
    }
    messageBuffer[messageSize++] = pCallsignSource->ssid;
    
    // TODO I do not encode digipeaters path (no idea if we need it at all)

    messageBuffer[messageSize++] = '\x3E'; /// 03
    messageBuffer[messageSize++] = '\xF0';

    // TODO I do not encode GPS for now

    // 2 bytes are dedicated to FCS
    generateFcs(&messageBuffer[PREFIX_FLAGS_COUNT], messageSize - PREFIX_FLAGS_COUNT, &messageBuffer[messageSize - PREFIX_FLAGS_COUNT + 1]);
    messageSize += 2;

    for (uint8_t i = 0; i < SUFFIX_FLAGS_COUNT; ++i)
    {
        messageBuffer[messageSize++] = '\x7E';
    }

    return messageSize;
}

void advanceBitstreamBit(struct BitstreamPos* pResultBitstreamSize)
{
    if (pResultBitstreamSize->bitstreamCharBitIdx == 7)
    {
        ++pResultBitstreamSize->bitstreamCharIdx;
        pResultBitstreamSize->bitstreamCharBitIdx = 0;
    }
    else
    {
        ++pResultBitstreamSize->bitstreamCharBitIdx;
    }
}

bool copyByte(struct EncodingData* pData,
              const uint8_t* pMessage,
              uint16_t messageIdx,
              uint8_t* pBistream,
              uint16_t bitStreamMaxLen,
              bool handleOnes)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        const uint8_t currentBit = pMessage[messageIdx] & (1 << i);

        if (currentBit)
        {
            if (pData->bitstreamSize.bitstreamCharIdx >= bitStreamMaxLen)
            {
                return false;
            }

            // as we are encoding one keep current bit as is
            if (pData->lastBit)
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (pData->bitstreamSize.bitstreamCharBitIdx);
            }
            else
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pData->bitstreamSize.bitstreamCharBitIdx));
            }

            advanceBitstreamBit(&pData->bitstreamSize);

            if (handleOnes)
            {
                ++pData->numberOfOnes;
                
                if (pData->numberOfOnes == 5)
                {
                    if (pData->bitstreamSize.bitstreamCharIdx >= bitStreamMaxLen)
                    {
                        return false;
                    }
                    // we need to insert 0
                    if (pData->lastBit)
                    {
                        pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pData->bitstreamSize.bitstreamCharBitIdx));
                        pData->lastBit = 0;
                    }
                    else
                    {
                        pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (pData->bitstreamSize.bitstreamCharBitIdx);
                        pData->lastBit = 1;
                    }
                    pData->numberOfOnes = 0;
                    advanceBitstreamBit(&pData->bitstreamSize); // insert zero as we had 5 ones
                }
            }
        }
        else
        {
            if (pData->bitstreamSize.bitstreamCharIdx >= bitStreamMaxLen)
            {
                return false;
            }
            // as we are encoding 0 we need to flip bit
            if (pData->lastBit)
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pData->bitstreamSize.bitstreamCharBitIdx));
                pData->lastBit = 0;
            }
            else
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (pData->bitstreamSize.bitstreamCharBitIdx);
                pData->lastBit = 1;
            }

            advanceBitstreamBit(&pData->bitstreamSize);

            if (handleOnes)
            {
                pData->numberOfOnes = 0;
            }
        }
    }
    return true;
}

bool encodeMessage2BitStream(const uint8_t* pMessage, 
                             uint16_t messageSize,
                             uint8_t* pBistream,
                             uint16_t bitstreamMaxLen,
                             struct BitstreamPos* pResultBitstreamSize)
{
    if (bitstreamMaxLen < messageSize || messageSize < 20)
    {
        return false;
    }

    struct EncodingData encodingData;

    encodingData.lastBit = 1;
    encodingData.numberOfOnes = 0;
    encodingData.bitstreamSize.bitstreamCharIdx = 0;
    encodingData.bitstreamSize.bitstreamCharBitIdx = 0;

    for (uint8_t i = 0; i < PREFIX_FLAGS_COUNT; ++i)
    {
        if (!copyByte(&encodingData, pMessage, i, pBistream, bitstreamMaxLen, false))
        {
            return false;
        }
    }
    for (uint8_t i = PREFIX_FLAGS_COUNT; i < messageSize - SUFFIX_FLAGS_COUNT; ++i)
    {
        if (!copyByte(&encodingData, pMessage, i, pBistream, bitstreamMaxLen, true))
        {
            return false;
        }
    }
    for (uint8_t i = messageSize - SUFFIX_FLAGS_COUNT; i < messageSize; ++i)
    {
        if (!copyByte(&encodingData, pMessage, i, pBistream, bitstreamMaxLen, false))
        {
            return false;
        }
    }

    *pResultBitstreamSize = encodingData.bitstreamSize;

    return true;
}

void createAprsMessage(const struct GpsData* pGpsData)
{
    g_currentBitstreamSize.bitstreamCharIdx = 0;
    g_currentBitstreamSize.bitstreamCharBitIdx = 0;

    g_currentBitstreamPos.bitstreamCharIdx = 0;
    g_currentBitstreamPos.bitstreamCharBitIdx = 0;

    g_currentF1200Frame = 0;
    g_currentF2200Frame = 0;
    g_currentFrequencyIsF1200 = true;
    g_currentSymbolPulsesCount = PULSES_PER_SYMBOL_COUNT;

    uint8_t message[APRS_MESSAGE_MAX_LEN];

    uint16_t messageSize = generateMessage(&CALLSIGN_DESTINATION,
                                           &CALLSIGN_SOURCE,
                                           message,
                                           APRS_MESSAGE_MAX_LEN);

    if (encodeMessage2BitStream(message, 
                                messageSize,
                                g_currentBitstream,
                                APRS_BITSTREAM_MAX_LEN,
                                &g_currentBitstreamSize))
    {
        g_sendingMessage = true;
    }
}

void enablePwm(void)
{
    ROM_PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    ROM_IntEnable(INT_PWM0_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

void disablePwm(void)
{
    ROM_PWMIntDisable(PWM0_BASE, PWM_INT_GEN_0);
    PWMGenIntTrigDisable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    ROM_IntDisable(INT_PWM0_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
    PWMGenDisable(PWM0_BASE, PWM_GEN_0);
}

void enableHx1(void)
{
    // TODO
}

void disableHx1(void)
{
    // TODO
}

float normalizePulseWidth(float width)
{
    if (width < PWM_MIN_PULSE_WIDTH)
    {
        return PWM_MIN_PULSE_WIDTH;
    }
    else if (width > PWM_MAX_PULSE_WIDTH)
    {
        return PWM_MAX_PULSE_WIDTH;
    }
    return width;
}

void Pwm10Handler(void)
{
    // TODO need to test different combinations of links between 1200Hz <-> 2200Hz to figure out
    //      where discontinuities are coming from
    
    // TODO adding 1 to g_currentF2200Frame calculation from pulseWidth1200
    
    // TODO I have reasons to believe that this code is too slow!!!

    // TODO I have reasons to believe that this code modulates signal with slightly different frequency when expected!!!
    
    // TODO it seems that it's better to disable UART (and other interrupts) while doing PWM

    // TODO plain modem doesn't require AFSK (1 is sent as 1 and zero as zero, also FCS should not be bit-inverted)
    
    // TODO to abort message send at least 15 ones no stuffing
    
    // TODO during IDLE time flag should be sent continously
    
    if (g_currentSymbolPulsesCount >= F1200_COUNT)
    {
        g_currentSymbolPulsesCount = 0;

        if (!g_sendingMessage || (g_currentBitstreamPos.bitstreamCharIdx >= g_currentBitstreamSize.bitstreamCharIdx && 
                                  g_currentBitstreamPos.bitstreamCharBitIdx >= g_currentBitstreamSize.bitstreamCharBitIdx))
        {
            disablePwm();
            disableHx1();
            g_sendingMessage = false;
            return;
        }
        
        const bool isOne = g_currentBitstream[g_currentBitstreamPos.bitstreamCharIdx] & (1 << g_currentBitstreamPos.bitstreamCharBitIdx);
    
        if (!isOne && g_currentFrequencyIsF1200)
        {
            const float trigaArg = ANGULAR_FREQUENCY_F1200 * g_currentF1200Frame;
            const float pulseWidth1200 = normalizePulseWidth(AMPLITUDE_SHIFT + AMPLITUDE_SCALER * sinf(trigaArg));
            const float pulseDirection1200 = cosf(trigaArg);

            if (pulseDirection1200 >= 0)
            {
                g_currentF2200Frame = RECIPROCAL_ANGULAR_FREQUENCY_F2200 * asinf(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth1200 - AMPLITUDE_SHIFT));
            }
            else
            {
                g_currentF2200Frame = HALF_PERIOD_F2200 - RECIPROCAL_ANGULAR_FREQUENCY_F2200 * asinf(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth1200 - AMPLITUDE_SHIFT));
            }
            
            g_currentFrequencyIsF1200 = false;
        }
        else if (!isOne && !g_currentFrequencyIsF1200)
        {
            const float trigArg = ANGULAR_FREQUENCY_F2200 * g_currentF2200Frame;
            const float pulseWidth2200 = normalizePulseWidth(AMPLITUDE_SHIFT + AMPLITUDE_SCALER * sinf(trigArg));
            const float pulseDirection2200 = cosf(trigArg);

            if (pulseDirection2200 >= 0)
            {
                g_currentF1200Frame = RECIPROCAL_ANGULAR_FREQUENCY_F1200 * asinf(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth2200 - AMPLITUDE_SHIFT));
            }
            else
            {
                g_currentF1200Frame = HALF_PERIOD_F1200 - RECIPROCAL_ANGULAR_FREQUENCY_F1200 * asinf(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth2200 - AMPLITUDE_SHIFT));
            }

            g_currentFrequencyIsF1200 = true;
        }
        
        advanceBitstreamBit(&g_currentBitstreamPos);
    }

    if (g_currentFrequencyIsF1200)
    {
        const uint32_t pulseWidth = (uint32_t) (AMPLITUDE_SHIFT_UINT + AMPLITUDE_SCALER * sinf(ANGULAR_FREQUENCY_F1200 * g_currentF1200Frame));
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pulseWidth);
        g_currentF1200Frame += PWM_STEP_SIZE;
        
        if (g_currentF1200Frame >= F1200_COUNT)
        {
            g_currentF1200Frame -= F1200_COUNT;
        }
    }
    else
    {
        const uint32_t pulseWidth = (uint32_t) (AMPLITUDE_SHIFT_UINT + AMPLITUDE_SCALER * sinf(ANGULAR_FREQUENCY_F2200 * g_currentF2200Frame));
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pulseWidth);
        g_currentF2200Frame += PWM_STEP_SIZE;
        if (g_currentF2200Frame >= F2200_COUNT)
        {
            g_currentF2200Frame -= F2200_COUNT;
        }
    }
    
    ++g_currentSymbolPulsesCount;
    
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
}
