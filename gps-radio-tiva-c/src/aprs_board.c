#include "aprs_board.h"
#include "aprs_board_impl.h"

#include <cmath>
#include <string.h>
#include <cstdio>

#include "uart.h"
#include "timer.h"
#include "tiva_c.h"
#include "common.h"
#include "generated_trig_data.h"

#ifdef TRIG_SLOW
    #define SINE(v)            sinf(v)
    #define COSINE_G_THAN_0(v) (cosf(v) >= 0)
    #define INVERSE_SINE(v)    asinf(v)
#else
    #define TRIG_FLOAT_TO_INT(value) \
        (int) ((value) * TRIG_MULTIPLIER + 0.5f)
            
    #define COS_0A TRIG_FLOAT_TO_INT(PI / 2.0f)
    #define COS_0B TRIG_FLOAT_TO_INT(3.0f * PI / 2.0f)
        
    #define INVERSE_TRIG_FLOAT_TO_INT(value) \
        (INVERSE_TRIG_MULTIPLIER + (int) ((value) * INVERSE_TRIG_MULTIPLIER + 0.5f))

    // this complication is due to the fact that mVision has a 32K limit for code
    // need to try to compile this stuff using Ubuntu/gcc and use normal table for cosine
    float cosineSign(int idx)
    {
        if ((idx >= 0 && idx <= COS_0A) ||
            (idx >= COS_0B))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    #define SINE(v)            SIN[TRIG_FLOAT_TO_INT(v)]
    #define COSINE_G_THAN_0(v) cosineSign(TRIG_FLOAT_TO_INT(v))
    #define INVERSE_SINE(v)    ASIN[INVERSE_TRIG_FLOAT_TO_INT(v)]
#endif

const Callsign CALLSIGN_SOURCE = 
{
    {"HABHAB"},
    '\xF6' // 111 1011 0
           //          ^ not a last address
           //     ^^^^ SSID (11 - balloon)
           // ^^^ some reserved values and command/response
};

const Callsign CALLSIGN_DESTINATION_1 = 
{
    {"WIDE1 "},
    '\xE2' // 111 0001 0
           //          ^ not a last address
           //     ^^^^ SSID (1 - wide1-1)
           // ^^^ some reserved values and command/response
};

const Callsign CALLSIGN_DESTINATION_2 = 
{
    {"WIDE2 "},
    '\xE5' // 111 0010 1
           //          ^ last address
           //     ^^^^ SSID (2 - wide2-2)
           // ^^^ some reserved values and command/response
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

static uint8_t g_aprsPayloadBuffer[APRS_PAYLOAD_LEN];

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

uint8_t createPacketPayload(const GpsData* pGpsData, const Telemetry* pTelemetry, uint8_t* pBuffer, uint8_t bufferSize)
{
    uint8_t bufferStartIdx = 0;

    if (pGpsData->isValid)
    {
        if (pGpsData->utcTime.isValid)
        {
            bufferStartIdx += sprintf((char*) &pBuffer[bufferStartIdx],
                                      "@%02i%02i%02iz",
                                      pGpsData->utcTime.hours,
                                      pGpsData->utcTime.minutes,
                                      (int) pGpsData->utcTime.seconds);
        }
        else
        {
            pBuffer[bufferStartIdx++] = '!';
        }
        
        // TODO add lat/long/course/etc
    }

    bufferStartIdx += sprintf((char*) &pBuffer[bufferStartIdx], "T#%03u,%03u", pTelemetry->cpuTemperature / 10, pTelemetry->voltage / 10);
    
    return bufferStartIdx;
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
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &CALLSIGN_DESTINATION_1.ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, pCallsignSource->callsign, 6, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &pCallsignSource->ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, CALLSIGN_DESTINATION_2.callsign, 6, PERFORM_STUFFING, CALCULATE_FCS, SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, &CALLSIGN_DESTINATION_2.ssid, 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);

    // control bytes
    
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\x03", 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, (const uint8_t*) "\xF0", 1, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);

    // packet contents
    
    const uint8_t bufferSize = createPacketPayload(pGpsData, pTelemetry, g_aprsPayloadBuffer, APRS_PAYLOAD_LEN);
    if (bufferSize == 0)
    {
        return false;
    }
#ifdef DUMP_DATA_TO_UART0
    writeString(CHANNEL_OUTPUT, "aprs - ");
    if (!writeMessageBuffer(CHANNEL_OUTPUT, g_aprsPayloadBuffer, bufferSize))
    {
        return false;
    }
    writeString(CHANNEL_OUTPUT, "\r\n");
#endif
    encodeAndAppendBits(bitstreamBuffer, maxBitstreamBufferLen, &encodingData, g_aprsPayloadBuffer, bufferSize, PERFORM_STUFFING, CALCULATE_FCS, NO_SHIFT_ONE_LEFT);
    
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
            else if (g_leadingOnesLeft)
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
                    const float triagArg = ANGULAR_FREQUENCY_F1200 * g_currentF1200Frame;
                    const float pulseWidth1200 = normalizePulseWidth(AMPLITUDE_SHIFT + AMPLITUDE_SCALER * SINE(triagArg));
                    const bool pulse1200Positive = COSINE_G_THAN_0(triagArg);

                    if (pulse1200Positive)
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
                    const bool pulse2200Positive = COSINE_G_THAN_0(trigArg);
                    
                    if (pulse2200Positive)
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
