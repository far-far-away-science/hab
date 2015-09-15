#include "aprs_board.h"

#include <cmath>
#include <string.h>
#include <cstdio>

#include "uart.h"
#include "timer.h"
#include "tiva_c.h"
#include "common.h"
#include "generated_trig_data.h"

#define PREFIX_FLAGS_COUNT 1
#define SUFFIX_FLAGS_COUNT 23

#define APRS_BITSTREAM_MAX_LEN 386 // bitstream will have extra bits in it so it must be larger than message buffer
                                   // in worst case we will insert extra 0 for every 5 bits

/*
 * FCS
 */
 
#define FCS_POLYNOMIAL 0x8408
#define FCS_INITIAL_VALUE 0xFFFF
#define FCS_POST_PROCESSING_XOR_VALUE 0xFFFF

/*
 * PWM
 */

/*
 * those values are calculated in advance depending on MCU/etc
 */

#define PI 3.141592654f

#define PWM_STEP_SIZE 1

#define PWM_PERIOD 650
#define PWM_MIN_PULSE_WIDTH 1
#define PWM_MAX_PULSE_WIDTH 647

#define F1200_PWM_PULSES_COUNT_PER_SYMBOL 64

// should be around 0.5ms for HX-1 warmup (10 / 1200 = 8ms)
#define LEADING_WARMUP_AMPLITUDE_DC_PULSES_COUNT 10
// to abord previous frame send at least 15 ones without any stuffing (putting zeroes in between)
#define LEADING_ONES_COUNT_TO_CANCEL_PREVIOUS_PACKET 48

/*
 * those values are calculated from prevous ones
 */

#define F2200_PWM_PULSES_COUNT_PER_SYMBOL (1200.0f * F1200_PWM_PULSES_COUNT_PER_SYMBOL / 2200.0f)

#define AMPLITUDE_SCALER ((float) (PWM_MAX_PULSE_WIDTH - PWM_MIN_PULSE_WIDTH) / 2.0f)
#define RECIPROCAL_AMPLITUDE_SCALER (1.0f / AMPLITUDE_SCALER)

#define AMPLITUDE_SHIFT ((float) (AMPLITUDE_SCALER + 1.0f))

#define HALF_PERIOD_F1200 (F1200_PWM_PULSES_COUNT_PER_SYMBOL / 2.0f)
#define HALF_PERIOD_F2200 (F2200_PWM_PULSES_COUNT_PER_SYMBOL / 2.0f)

#define ANGULAR_FREQUENCY_F1200 (2.0f * PI / F1200_PWM_PULSES_COUNT_PER_SYMBOL)
#define ANGULAR_FREQUENCY_F2200 (2200.0f * ANGULAR_FREQUENCY_F1200 / 1200.0f)

#define RECIPROCAL_ANGULAR_FREQUENCY_F1200 (1.0f / ANGULAR_FREQUENCY_F1200) 
#define RECIPROCAL_ANGULAR_FREQUENCY_F2200 (1.0f / ANGULAR_FREQUENCY_F2200) 

#ifdef TRIG_SLOW
    #define SINE(v)         sinf(v)
    #define COSINE(v)       cosf(v)
    #define INVERSE_SINE(v) asinf(v)
#else
    #define TRIG_FLOAT_TO_INT(value) \
        (int) ((value) * TRIG_MULTIPLIER + 0.5f)
            
    #define COS_SHIFT TRIG_FLOAT_TO_INT(PI / 2.0f)
        
    #define INVERSE_TRIG_FLOAT_TO_INT(value) \
        (INVERSE_TRIG_MULTIPLIER + (int) ((value) * INVERSE_TRIG_MULTIPLIER + 0.5f))

    // this complication is due to the fact that mVision has a 32K limit for code
    // need to try to compile this stuff using Ubuntu/gcc and use normal table for cosine
    float cosine(int idx)
    {
        idx += COS_SHIFT;
        if (idx < TRIG_COUNT)
        {
            return SIN[idx];
        }
        else
        {
            return SIN[idx - TRIG_COUNT];
        }
    }
        
    #define SINE(v)         SIN[TRIG_FLOAT_TO_INT(v)]
    #define COSINE(v)       cosine(TRIG_FLOAT_TO_INT(v))
    #define INVERSE_SINE(v) ASIN[INVERSE_TRIG_FLOAT_TO_INT(v)]
#endif

typedef enum FCS_TYPE_t
{
    NO_FCS,
    CALCULATE_FCS,
} FCS_TYPE;

typedef enum STUFFING_TYPE_t
{
    NO_STUFFING,
    PERFORM_STUFFING,
} STUFFING_TYPE;

typedef enum SHIFT_ONE_LEFT_TYPE_t
{
    NO_SHIFT_ONE_LEFT,
    SHIFT_ONE_LEFT,
} SHIFT_ONE_LEFT_TYPE;

typedef struct BitstreamPos_t
{
    uint16_t bitstreamCharIdx;
    uint8_t bitstreamCharBitIdx;
} BitstreamPos;

typedef struct EncodingData_t
{
    uint16_t fcs;
    uint8_t lastBit;
    uint8_t numberOfOnes;
    BitstreamPos bitstreamSize;
} EncodingData;

const Callsign CALLSIGN_SOURCE = 
{
    {"HABHAB"},
    0x61
};

const Callsign CALLSIGN_DESTINATION_1 = 
{
    {"WIDE1-"},
    '1'
};

const Callsign CALLSIGN_DESTINATION_2 = 
{
    {"WIDE2-"},
    '2'
};

bool g_sendingMessage = false;

uint16_t g_leadingOnesLeft = 0;
uint16_t g_leadingWarmUpLeft = 0;
BitstreamPos g_currentBitstreamPos = { 0 };
BitstreamPos g_currentBitstreamSize = { 0 };
uint8_t g_currentBitstream[APRS_BITSTREAM_MAX_LEN] = { 0 };

bool g_currentFrequencyIsF1200 = true;

float g_currentF1200Frame = 0;
float g_currentF2200Frame = 0;
uint8_t g_currentSymbolPulsesCount = 0;

bool createAprsMessage(const GpsData* pGpsData, const Telemetry* pTelemetry);

void initializeAprs(void)
{
    initializeAprsHardware(PWM_PERIOD, PWM_MIN_PULSE_WIDTH);
}

bool sendAprsMessage(const GpsData* pGpsData, const Telemetry* pTelemetry)
{
    if (g_sendingMessage)
    {
        return false;
    }
    if (!createAprsMessage(pGpsData, pTelemetry))
    {
        return false;
    }
    enableHx1();
    enableAprsPwm();
    return true;
}

void advanceBitstreamBit(BitstreamPos* pResultBitstreamSize)
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
bool encodeAndAppendBits(uint8_t* pBitstreamBuffer,
                         uint16_t maxBitstreamBufferLen,
                         EncodingData* pEncodingData,
                         const uint8_t* pMessageData,
                         uint16_t messageDataSize,
                         STUFFING_TYPE stuffingType,
                         FCS_TYPE fcsType,
                         SHIFT_ONE_LEFT_TYPE shiftOneLeftType)
{
    if (!pBitstreamBuffer || !pEncodingData || maxBitstreamBufferLen < messageDataSize)
    {
        return false;
    }        
    if (messageDataSize == 0)
    {
        return true;
    }
    if (!pMessageData)
    {
        return true;
    }
    
    for (uint16_t iByte = 0; iByte < messageDataSize; ++iByte)
    {
        uint8_t currentByte = pMessageData[iByte];
        
        if (shiftOneLeftType == SHIFT_ONE_LEFT)
        {
            currentByte <<= 1;
        }
        
        for (uint8_t iBit = 0; iBit < 8; ++iBit)
        {
            const uint8_t currentBit = currentByte & (1 << iBit);

            if (fcsType == CALCULATE_FCS)
            {
                const uint16_t shiftBit = pEncodingData->fcs & 0x0001;
                pEncodingData->fcs = pEncodingData->fcs >> 1;
                if (shiftBit != ((currentByte >> iBit) & 0x01))
                {
                    pEncodingData->fcs ^= FCS_POLYNOMIAL;
                }
            }

            if (currentBit)
            {
                if (pEncodingData->bitstreamSize.bitstreamCharIdx >= maxBitstreamBufferLen)
                {
                    return false;
                }
                // as we are encoding 1 keep current bit as is
                if (pEncodingData->lastBit)
                {
                    pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] |= 1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx);
                }
                else
                {
                    pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx));
                }

                advanceBitstreamBit(&pEncodingData->bitstreamSize);

                if (stuffingType == PERFORM_STUFFING)
                {
                    ++pEncodingData->numberOfOnes;
                    
                    if (pEncodingData->numberOfOnes == 5)
                    {
                        if (pEncodingData->bitstreamSize.bitstreamCharIdx >= maxBitstreamBufferLen)
                        {
                            return false;
                        }

                        // we need to insert 0 after 5 consecutive ones
                        if (pEncodingData->lastBit)
                        {
                            pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx));
                            pEncodingData->lastBit = 0;
                        }
                        else
                        {
                            pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] |= 1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx);
                            pEncodingData->lastBit = 1;
                        }
                        
                        pEncodingData->numberOfOnes = 0;
                        
                        advanceBitstreamBit(&pEncodingData->bitstreamSize); // insert zero as we had 5 ones
                    }
                }
            }
            else
            {
                if (pEncodingData->bitstreamSize.bitstreamCharIdx >= maxBitstreamBufferLen)
                {
                    return false;
                }
                
                // as we are encoding 0 we need to flip bit
                if (pEncodingData->lastBit)
                {
                    pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] &= ~(1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx));
                    pEncodingData->lastBit = 0;
                }
                else
                {
                    pBitstreamBuffer[pEncodingData->bitstreamSize.bitstreamCharIdx] |= 1 << (pEncodingData->bitstreamSize.bitstreamCharBitIdx);
                    pEncodingData->lastBit = 1;
                }

                advanceBitstreamBit(&pEncodingData->bitstreamSize);

                if (stuffingType == PERFORM_STUFFING)
                {
                    pEncodingData->numberOfOnes = 0;
                }
            }
        }
    }

    if (stuffingType == NO_STUFFING)
    {
        // resert ones as we didn't do any stuffing while sending this data
        pEncodingData->numberOfOnes = 0;
    }

    return true;
}

bool generateMessage(const Callsign* pCallsignSource,
                     const GpsData* pGpsData,
                     const Telemetry* pTelemetry,
                     uint8_t* bitstreamBuffer,
                     uint16_t maxBitstreamBufferLen,
                     BitstreamPos* pBitstreamSize)
{
    if (!pBitstreamSize || !pCallsignSource || !pGpsData || !bitstreamBuffer)
    {
        return false;
    }
    
    EncodingData encodingData = { 0 };
    encodingData.lastBit = 1;
    encodingData.fcs = FCS_INITIAL_VALUE;

    for (uint8_t i = 0; i < PREFIX_FLAGS_COUNT; ++i)
    {
        encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\x7E", 1, NO_STUFFING, NO_FCS, NO_SHIFT_ONE_LEFT);
    }

    // addresses to and from
    
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, CALLSIGN_DESTINATION_1.callsign, 6, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &CALLSIGN_DESTINATION_1.ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, pCallsignSource->callsign, 6, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &pCallsignSource->ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, CALLSIGN_DESTINATION_2.callsign, 6, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &CALLSIGN_DESTINATION_2.ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);

    // control bytes
    
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\x03", 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\xF0", 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);

    //
    //
    // TODO add GPS encoding
    // TODO temporary code below
    // TODO encode telemetry
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "Hello World!", 12, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    // TODO add GPS encoding
    //
    //

    // fcs

    encodingData.fcs ^= FCS_POST_PROCESSING_XOR_VALUE;
    uint8_t fcsByte = encodingData.fcs & 0x00FF; // get low byte
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &fcsByte, 1, PERFORM_STUFFING, NO_FCS, NO_SHIFT_ONE_LEFT);
    fcsByte = (encodingData.fcs >> 8) & 0x00FF; // get high byte
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &fcsByte, 1, PERFORM_STUFFING, NO_FCS, NO_SHIFT_ONE_LEFT);

    // sufix flags

    for (uint8_t i = 0; i < SUFFIX_FLAGS_COUNT; ++i)
    {
        encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\x7E", 1, NO_STUFFING, NO_FCS, NO_SHIFT_ONE_LEFT);
    }

    *pBitstreamSize = encodingData.bitstreamSize;

    return true;
}


bool createAprsMessage(const GpsData* pGpsData, const Telemetry* pTelemetry)
{
    g_leadingOnesLeft = LEADING_ONES_COUNT_TO_CANCEL_PREVIOUS_PACKET;
    g_leadingWarmUpLeft = LEADING_WARMUP_AMPLITUDE_DC_PULSES_COUNT;
    
    g_currentBitstreamSize.bitstreamCharIdx = 0;
    g_currentBitstreamSize.bitstreamCharBitIdx = 0;

    g_currentBitstreamPos.bitstreamCharIdx = 0;
    g_currentBitstreamPos.bitstreamCharBitIdx = 0;

    g_currentF1200Frame = 0;
    g_currentF2200Frame = 0;
    g_currentFrequencyIsF1200 = true;
    g_currentSymbolPulsesCount = F1200_PWM_PULSES_COUNT_PER_SYMBOL;

    if (generateMessage(&CALLSIGN_SOURCE,
                        pGpsData,
                        pTelemetry,
                        g_currentBitstream,
                        APRS_BITSTREAM_MAX_LEN,
                        &g_currentBitstreamSize))
    {
        g_sendingMessage = true;
        return true;
    }
    else
    {
        return false;
    }
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

char buffer[128];

void Pwm10Handler(void)
{
    clearAprsPwmInterrupt();
    
    if (g_leadingWarmUpLeft)
    {
        // make sure HX1 has a chance to warm up
        setAprsPwmPulseWidth(PWM_MIN_PULSE_WIDTH);
        --g_leadingWarmUpLeft;
    }
    else
    {
        if (g_currentSymbolPulsesCount >= F1200_PWM_PULSES_COUNT_PER_SYMBOL)
        {
            g_currentSymbolPulsesCount = 0;

            if (!g_sendingMessage || (g_currentBitstreamPos.bitstreamCharIdx >= g_currentBitstreamSize.bitstreamCharIdx && 
                                      g_currentBitstreamPos.bitstreamCharBitIdx >= g_currentBitstreamSize.bitstreamCharBitIdx))
            {
                disableAprsPwm();
                disableHx1();
                setAprsPwmPulseWidth(PWM_MIN_PULSE_WIDTH);
                g_sendingMessage = false;
                return;
            }
            else if (g_leadingOnesLeft > 0)
            {
                // send ones to stabilize HX1 and cancel any previosuly not-fully received APRS packets
                g_currentFrequencyIsF1200 = true;
                --g_leadingOnesLeft;
            }
            else
            {
                // bit stream is already AFSK encoded so we simply send ones and zeroes as is
                const bool isOne = g_currentBitstream[g_currentBitstreamPos.bitstreamCharIdx] & (1 << g_currentBitstreamPos.bitstreamCharBitIdx);

                // make sure new zero bit frequency is 2200
                if (!isOne && g_currentFrequencyIsF1200)
                {
                    const float trigaArg = ANGULAR_FREQUENCY_F1200 * g_currentF1200Frame;
                    const float pulseWidth1200 = normalizePulseWidth(AMPLITUDE_SHIFT + AMPLITUDE_SCALER * SINE(trigaArg));
                    const float pulseDirection1200 = COSINE(trigaArg);

                    if (pulseDirection1200 >= 0)
                    {
                        g_currentF2200Frame = RECIPROCAL_ANGULAR_FREQUENCY_F2200 * INVERSE_SINE(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth1200 - AMPLITUDE_SHIFT));
                    }
                    else
                    {
                        g_currentF2200Frame = HALF_PERIOD_F2200 - RECIPROCAL_ANGULAR_FREQUENCY_F2200 * INVERSE_SINE(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth1200 - AMPLITUDE_SHIFT));
                    }
                    
                    if (g_currentF2200Frame < 0)
                    {
                        g_currentF2200Frame += F2200_PWM_PULSES_COUNT_PER_SYMBOL;
                    }

                    g_currentFrequencyIsF1200 = false;
                }
                // make sure new one bit frequency is 1200
                else if (isOne && !g_currentFrequencyIsF1200)
                {
                    const float trigArg = ANGULAR_FREQUENCY_F2200 * g_currentF2200Frame;
                    const float pulseWidth2200 = normalizePulseWidth(AMPLITUDE_SHIFT + AMPLITUDE_SCALER * SINE(trigArg));
                    const float pulseDirection2200 = COSINE(trigArg);
                    
                    if (pulseDirection2200 >= 0)
                    {
                        g_currentF1200Frame = RECIPROCAL_ANGULAR_FREQUENCY_F1200 * INVERSE_SINE(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth2200 - AMPLITUDE_SHIFT));
                    }
                    else
                    {
                        g_currentF1200Frame = HALF_PERIOD_F1200 - RECIPROCAL_ANGULAR_FREQUENCY_F1200 * INVERSE_SINE(RECIPROCAL_AMPLITUDE_SCALER * (pulseWidth2200 - AMPLITUDE_SHIFT));
                    }

                    if (g_currentF1200Frame < 0)
                    {
                        g_currentF1200Frame += F1200_PWM_PULSES_COUNT_PER_SYMBOL;
                    }

                    g_currentFrequencyIsF1200 = true;
                }
                
                advanceBitstreamBit(&g_currentBitstreamPos);
            }
        }

        if (g_currentFrequencyIsF1200)
        {
            const uint32_t pulseWidth = (uint32_t) (AMPLITUDE_SHIFT + AMPLITUDE_SCALER * SINE(ANGULAR_FREQUENCY_F1200 * g_currentF1200Frame));
            setAprsPwmPulseWidth(pulseWidth);
            g_currentF1200Frame += PWM_STEP_SIZE;
            if (g_currentF1200Frame >= F1200_PWM_PULSES_COUNT_PER_SYMBOL)
            {
                g_currentF1200Frame -= F1200_PWM_PULSES_COUNT_PER_SYMBOL;
            }
        }
        else
        {
            const uint32_t pulseWidth = (uint32_t) (AMPLITUDE_SHIFT + AMPLITUDE_SCALER * SINE(ANGULAR_FREQUENCY_F2200 * g_currentF2200Frame));
            setAprsPwmPulseWidth(pulseWidth);
            g_currentF2200Frame += PWM_STEP_SIZE;
            if (g_currentF2200Frame >= F2200_PWM_PULSES_COUNT_PER_SYMBOL)
            {
                g_currentF2200Frame -= F2200_PWM_PULSES_COUNT_PER_SYMBOL;
            }
        }
        
        ++g_currentSymbolPulsesCount;
    }
}
