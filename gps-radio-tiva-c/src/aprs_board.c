#include "aprs_board.h"

#include <string.h>

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

#define F1200_COUNT 59
#define F2200_COUNT 32

#define MAX_SYMBOL_PULSES_COUNT 64

#define APRS_MESSAGE_MAX_LEN   256
#define APRS_BITSTREAM_MAX_LEN 384 // bitstream will have extra bits in it so it must be larger than message buffer
                                   // in worst case we will insert extra 0 for every 5 bits

struct BitstreamSize
{
    uint16_t bitstreamCharIdx;
    uint8_t bitstreamCharBitIdx;
};

struct EncodingData
{
    uint8_t lastBit;
    uint8_t numberOfOnes;
    struct BitstreamSize bitstreamSize;
};

const uint16_t F1200_DATA[] =
{
    354, 386, 418, 450, 480, 509, 536, 560, 583, 603, 620, 634, 645, 652, 656, 657, 654, 648, 638, 625, 609, 590, 568, 544, 518, 490, 460, 429, 397, 365, 332, 300, 269, 238, 209, 181, 156, 132, 111, 93, 78, 67, 58, 53, 51, 53, 58, 67, 78, 93, 111, 132, 156, 181, 209, 238, 269, 300, 332
};

const uint8_t F1200_2_F2200[] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 8, 11, 11, 12, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 24, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 31, 31, 0, 0
};

const uint16_t F2200_DATA[] =
{
    354, 423, 489, 550, 604, 648, 680, 700, 707, 700, 680, 648, 604, 550, 489, 423, 354, 285, 219, 158, 104, 60, 28, 8, 1, 8, 28, 60, 104, 158, 219, 285
};

const uint8_t F2200_2_F1200[] =
{
    0, 3, 5, 7, 10, 13, 15, 15, 15, 16, 16, 17, 21, 23, 26, 28, 30, 32, 34, 36, 39, 42, 44, 44, 44, 45, 45, 47, 50, 53, 55, 57
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

uint16_t g_currentBitstreamCharIdx = 0;
uint8_t  g_currentBitstreamCharBitIdx = 0;
struct BitstreamSize g_currentBitstreamSize = { 0 };
uint8_t  g_currentBitstream[APRS_BITSTREAM_MAX_LEN] = { 0 };

bool g_currentFrequencyIsF1200 = true;

uint8_t g_currentF1200Frame = 0;
uint8_t g_currentF2200Frame = 0;
uint8_t g_currentSymbolPulsesCount = 0;

void initializeAprs(void)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 710);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 710 / 2);
}

void generateFcs(uint8_t* pMessage, uint16_t messageSize, uint8_t* pFcs)
{
    uint16_t fcs = 0xFFFF;
    for (uint16_t i = 0; i < messageSize; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            const uint16_t shiftBit = fcs & 0x0001;
            fcs = fcs >> 1;
            if (shiftBit != (pMessage[i] & (1 << j)))
            {
                fcs ^= 0x8408;
            }
        }
    }
    fcs ^= 0xffff;
    pFcs[0] = fcs & 0x00FF;
    pFcs[1] = (fcs >> 8) & 0x00FF;
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
    
    messageBuffer[messageSize++] = '\x7E';

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

    messageBuffer[messageSize++] = '\x3E';
    messageBuffer[messageSize++] = '\xF0';

    // bits 17, 18 are FCS
    generateFcs(&messageBuffer[1], 16, &messageBuffer[17]);
    messageSize += 2;

    messageBuffer[messageSize++] = '\x7E';

    return messageSize;
}

void advanceBitstreamBit(struct BitstreamSize* pResultBitstreamSize)
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

            if (pData->lastBit)
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx);
            }
            else
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx));
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
                    if (pData->lastBit)
                    {
                        pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx));
                        pData->lastBit = 0;
                    }
                    else
                    {
                        pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx);
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
            if (pData->lastBit)
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx));
                pData->lastBit = 0;
            }
            else
            {
                pBistream[pData->bitstreamSize.bitstreamCharIdx] |= 1 << (7 - pData->bitstreamSize.bitstreamCharBitIdx);
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
                             struct BitstreamSize* pResultBitstreamSize)
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

    if (!copyByte(&encodingData, pMessage, 0, pBistream, bitstreamMaxLen, false))
    {
        return false;
    }
    for (uint8_t i = 1; i < messageSize - 1; ++i)
    {
        if (!copyByte(&encodingData, pMessage, i, pBistream, bitstreamMaxLen, true))
        {
            return false;
        }
    }
    if (!copyByte(&encodingData, pMessage, messageSize - 1, pBistream, bitstreamMaxLen, false))
    {
        return false;
    }

    *pResultBitstreamSize = encodingData.bitstreamSize;

    return true;
}

void createAprsMessage(const struct GpsData* pGpsData)
{
    g_currentBitstreamCharIdx = 0;
    g_currentBitstreamCharBitIdx = 0;
    g_currentBitstreamSize.bitstreamCharIdx = 0;
    g_currentBitstreamSize.bitstreamCharBitIdx = 0;

    g_currentF1200Frame = 0;
    g_currentF2200Frame = 0;
    g_currentFrequencyIsF1200 = true;
    g_currentSymbolPulsesCount = MAX_SYMBOL_PULSES_COUNT;

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

void Pwm10Handler(void)
{
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);

    if (g_currentSymbolPulsesCount >= MAX_SYMBOL_PULSES_COUNT)
    {
        g_currentSymbolPulsesCount = 0;

        if (g_currentBitstreamCharBitIdx > 7)
        {
            ++g_currentBitstreamCharIdx;
            g_currentBitstreamCharBitIdx = 0;
        }

        if (!g_sendingMessage || (g_currentBitstreamCharIdx >= g_currentBitstreamSize.bitstreamCharIdx &&
                                  g_currentBitstreamCharBitIdx >= g_currentBitstreamSize.bitstreamCharBitIdx))
        {
            disablePwm();
            disableHx1();
            g_sendingMessage = false;
            return;
        }

        const bool isOne = g_currentBitstream[g_currentBitstreamCharIdx] & (1 << g_currentBitstreamCharBitIdx);
        
        if (isOne && !g_currentFrequencyIsF1200)
        {
            g_currentF1200Frame = F2200_2_F1200[g_currentF2200Frame];
            g_currentFrequencyIsF1200 = true;
        }
        else if (!isOne && g_currentFrequencyIsF1200)
        {
            g_currentF2200Frame = F1200_2_F2200[g_currentF1200Frame];
            g_currentFrequencyIsF1200 = false;
        }
        
        ++g_currentBitstreamCharBitIdx;
    }
    
    if (g_currentFrequencyIsF1200)
    {
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, F1200_DATA[g_currentF1200Frame]);
        ++g_currentF1200Frame;
        if (g_currentF1200Frame >= F1200_COUNT)
        {
            g_currentF1200Frame = 0;
        }
    }
    else
    {
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, F2200_DATA[g_currentF2200Frame]);
        ++g_currentF2200Frame;
        if (g_currentF2200Frame >= F2200_COUNT)
        {
            g_currentF2200Frame = 0;
        }
    }
    
    ++g_currentSymbolPulsesCount;
}
