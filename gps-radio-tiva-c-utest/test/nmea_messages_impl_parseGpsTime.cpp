#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseGpsTime), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseGpsTime(MAKE_CONTEXT("", 10), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseGpsTime(MAKE_CONTEXT("123", 10), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseGpsTime(MAKE_CONTEXT("", 0), &result));
            Assert::AreEqual(0U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_no_separator_until_end_of_message)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseGpsTime(MAKE_CONTEXT("143407.00", 0), &result));
            Assert::AreEqual(9U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_with_no_whole_seconds_and_4_fractional_digits_in_seconds)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("1434.7123,", 0), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_with_1_whole_second_and_4_fractional_digits_in_seconds)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("14341.7123,", 0), &result));
            Assert::AreEqual(11U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_with_only_one_digit_in_seconds)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("14347,", 0), &result));
            Assert::AreEqual(6U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_without_seconds)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("1434,", 0), &result));
            Assert::AreEqual(5U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_without_seconds_and_only_one_digit_in_minutes)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("143,", 0), &result));
            Assert::AreEqual(4U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_without_seconds_and_minutes)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("14,", 0), &result));
            Assert::AreEqual(3U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_normal_gps_time_without_seconds_and_minutes_and_only_one_digit_in_hours)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseGpsTime(MAKE_CONTEXT("1,", 0), &result));
            Assert::AreEqual(2U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_hours_overflow)
        {
            GpsTime result;
            Assert::AreEqual(NPR_OVERFLOW, ::parseGpsTime(MAKE_CONTEXT("251212.12,", 0), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_minutes_overflow)
        {
            GpsTime result;
            Assert::AreEqual(NPR_OVERFLOW, ::parseGpsTime(MAKE_CONTEXT("126012.12,", 0), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_seconds_overflow)
        {
            GpsTime result;
            Assert::AreEqual(NPR_OVERFLOW, ::parseGpsTime(MAKE_CONTEXT("121260.00,", 0), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Valid_empty_gps_time)
        {
            GpsTime result;
            result.isValid = true;
            Assert::AreEqual(NPR_EMPTY_VALUE, ::parseGpsTime(MAKE_CONTEXT(",", 0), &result));
            Assert::AreEqual(1U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Valid_normal_gps_time)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("143407.12,", 0), &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 712, result.seconds);
        }

        TEST_METHOD(Valid_normal_gps_time_without_fractional_seconds)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("143407,", 0), &result));
            Assert::AreEqual(7U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 700, result.seconds);
        }

        TEST_METHOD(Valid_normal_gps_time_with_partial_fractional_seconds)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("143407.1,", 0), &result));
            Assert::AreEqual(9U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 710, result.seconds);
        }

        TEST_METHOD(Valid_normal_gps_time_with_more_than_2_fractional_seconds)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("143407.12345678,", 0), &result));
            Assert::AreEqual(16U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 712, result.seconds);
        }

        TEST_METHOD(Valid_normal_gps_time_with_non_digit_characters_at_the_end)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("143407.12z,", 0), &result));
            Assert::AreEqual(11U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 712, result.seconds);
        }

        TEST_METHOD(Valid_lots_of_zeroes)
        {
            {
                GpsTime result;
                Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("000000,", 0), &result));
                Assert::AreEqual(7U, context.tokenStartIdx);
                Assert::IsTrue(result.isValid);
                Assert::AreEqual((uint8_t) 0, result.hours);
                Assert::AreEqual((uint8_t) 0, result.minutes);
                Assert::AreEqual((uint16_t) 0, result.seconds);
            }

            {
                GpsTime result;
                Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("000000.0,", 0), &result));
                Assert::AreEqual(9U, context.tokenStartIdx);
                Assert::IsTrue(result.isValid);
                Assert::AreEqual((uint8_t) 0, result.hours);
                Assert::AreEqual((uint8_t) 0, result.minutes);
                Assert::AreEqual((uint16_t) 0, result.seconds);
            }

            {
                GpsTime result;
                Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("000000.000,", 0), &result));
                Assert::AreEqual(11U, context.tokenStartIdx);
                Assert::IsTrue(result.isValid);
                Assert::AreEqual((uint8_t) 0, result.hours);
                Assert::AreEqual((uint8_t) 0, result.minutes);
                Assert::AreEqual((uint16_t) 0, result.seconds);
            }
        }

        TEST_METHOD(Valid_max_values)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("235959.999,", 0), &result));
            Assert::AreEqual(11U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 23, result.hours);
            Assert::AreEqual((uint8_t) 59, result.minutes);
            Assert::AreEqual((uint16_t) 5999, result.seconds);
        }

        TEST_METHOD(Valid_starts_at_non_zero)
        {
            GpsTime result;
            Assert::AreEqual(NPR_VALID, ::parseGpsTime(MAKE_CONTEXT("abc143407.12,", 3), &result));
            Assert::AreEqual(13U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 14, result.hours);
            Assert::AreEqual((uint8_t) 34, result.minutes);
            Assert::AreEqual((uint16_t) 712, result.seconds);
        }
    };
}
