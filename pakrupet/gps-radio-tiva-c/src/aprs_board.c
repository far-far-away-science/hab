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
#include "common.h"

#define F1200_COUNT 59
#define F2200_COUNT 32

#define MAX_SYMBOL_PULSES_COUNT 64

#define APRS_BUFFER_MAX_LEN 256

const uint16_t F1200_DATA[] =
{
    354,
    392,
    429,
    465,
    501,
    534,
    566,
    595,
    621,
    644,
    664,
    680,
    693,
    701,
    706,
    707,
    703,
    696,
    685,
    670,
    651,
    629,
    604,
    575,
    545,
    512,
    477,
    441,
    404,
    367,
    329,
    291,
    255,
    219,
    185,
    153,
    123,
    96,
    71,
    50,
    33,
    19,
    9,
    3,
    1,
    3,
    9,
    19,
    33,
    50,
    71,
    96,
    123,
    153,
    185,
    219,
    255,
    291,
    329
};

const uint8_t F1200_2_F2200[] =
{
    1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 8, 9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 0, 0
};

const uint16_t F2200_DATA[] =
{
    354,
    423,
    489,
    550,
    604,
    648,
    680,
    700,
    707,
    700,
    680,
    648,
    604,
    550,
    489,
    423,
    354,
    285,
    219,
    158,
    104,
    60,
    28,
    8,
    1,
    8,
    28,
    60,
    104,
    158,
    219,
    285
};

const uint8_t F2200_2_F1200[] = {
    1, 2, 4, 6, 8, 10, 12, 13, 15, 17, 19, 21, 23, 24, 26, 28, 30, 32, 34, 35, 37, 39, 41, 43, 44, 46, 48, 50, 52, 54, 56, 57
};

bool g_sendingMessage = false;

uint16_t g_currentMessageSize = 0;
uint16_t g_currentMessageWordIdx = 0;

uint8_t g_currentMessageCharBitIdx = 0;
uint8_t g_currentMessage[APRS_BUFFER_MAX_LEN];

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

void createAprsMessage(const struct GpsData* pGpsData)
{
    g_sendingMessage = true;

    g_currentF1200Frame = 0;
    g_currentF2200Frame = 0;
    g_currentFrequencyIsF1200 = true;
    g_currentSymbolPulsesCount = MAX_SYMBOL_PULSES_COUNT;

    g_currentMessageWordIdx = 0;
    g_currentMessageCharBitIdx = 0;
    
    // TODO implement APRS message creation
    // const uint8_t TEST_MSG[] = "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA";
    const uint8_t TEST_MSG[] = "\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC";
    // const uint8_t TEST_MSG[] = "\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE\xEE";
    // const uint8_t TEST_MSG[] = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    // const uint8_t TEST_MSG[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    g_currentMessageSize = 25;
    memcpy(g_currentMessage, TEST_MSG, g_currentMessageSize);
    // TODO end
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

        if (g_currentMessageCharBitIdx > 7)
        {
            ++g_currentMessageWordIdx;
            g_currentMessageCharBitIdx = 0;
        }

        if (!g_sendingMessage || g_currentMessageWordIdx >= g_currentMessageSize)
        {
            disablePwm();
            disableHx1();
            g_sendingMessage = false;
            return;
        }

        if (g_currentMessage[g_currentMessageWordIdx] & (1 << g_currentMessageCharBitIdx))
        {
            // one doesn't change current frequency
        }
        else
        {
            if (g_currentFrequencyIsF1200)
            {
                // make sure once we change to 2200Hz frequency it looks smooth
                g_currentF2200Frame = F1200_2_F2200[g_currentF1200Frame];
            }
            else
            {
                // make sure one we change to 1200Hz frequency it looks smooth
                g_currentF1200Frame = F2200_2_F1200[g_currentF2200Frame];
            }
            // zero changes current freuqncy to opposite one
            g_currentFrequencyIsF1200 = !g_currentFrequencyIsF1200;
        }
        
        ++g_currentMessageCharBitIdx;
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
