#pragma once

#include "aprs_board.h"

#define PREFIX_FLAGS_COUNT 1
#define SUFFIX_FLAGS_COUNT 10

#define APRS_BITSTREAM_MAX_LEN 386 // bitstream will have extra bits in it so it must be larger than message buffer
                                   // in worst case we will insert extra 0 for every 5 bits

#define APRS_PAYLOAD_LEN 128

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

typedef enum FCS_TYPE_t
{
    FCS_NONE,
    FCS_CALCULATE,
} FCS_TYPE;

typedef enum STUFFING_TYPE_t
{
    ST_NO_STUFFING,
    ST_PERFORM_STUFFING,
} STUFFING_TYPE;

typedef enum SHIFT_ONE_LEFT_TYPE_t
{
    SHIFT_ONE_LEFT_NO,
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

void advanceBitstreamBit(BitstreamPos* pResultBitstreamSize);

float normalizePulseWidth(float width);

bool encodeAndAppendBits(uint8_t* pBitstreamBuffer,
                         uint16_t maxBitstreamBufferLen,
                         EncodingData* pEncodingData,
                         const uint8_t* pMessageData,
                         uint16_t messageDataSize,
                         STUFFING_TYPE stuffingType,
                         FCS_TYPE fcsType,
                         SHIFT_ONE_LEFT_TYPE shiftOneLeftType);

uint8_t createPacketPayload(GpsDataSource gpsDataSource, const GpsData* pGpsData, const Telemetry* pTelemetry, uint16_t messageIdx, uint8_t* pBuffer, uint8_t bufferSize);

bool generateMessage(const Callsign* pCallsignSource,
                     GpsDataSource gpsDataSource,
                     const GpsData* pGpsData,
                     const Telemetry* pTelemetry,
                     uint8_t* bitstreamBuffer,
                     uint16_t maxBitstreamBufferLen,
                     BitstreamPos* pBitstreamSize);

void Pwm10Handler(void);
