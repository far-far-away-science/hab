#include "..\..\stdafx.h"

#include "aprs_board_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(aprs_board_test_normalizePulseWidth)
    {
        TEST_METHOD(Should_return_min_pulse_width_if_pulse_is_less_than_min_pulse_width)
        {
            Assert::AreEqual((float) PWM_MIN_PULSE_WIDTH, normalizePulseWidth(PWM_MIN_PULSE_WIDTH - 1.0f));
        }

        TEST_METHOD(Should_return_max_pulse_width_if_pulse_is_greater_than_max_pulse_width)
        {
            Assert::AreEqual((float) PWM_MAX_PULSE_WIDTH, normalizePulseWidth(PWM_MAX_PULSE_WIDTH + 1.0f));
        }

        TEST_METHOD(Should_return_pulse_width_as_is_if_its_between_min_and_max)
        {
            Assert::AreEqual(PWM_MIN_PULSE_WIDTH + 1.0f, normalizePulseWidth(PWM_MIN_PULSE_WIDTH + 1.0f));
            Assert::AreEqual(PWM_MAX_PULSE_WIDTH - 1.0f, normalizePulseWidth(PWM_MAX_PULSE_WIDTH - 1.0f));
        }
    };
}
