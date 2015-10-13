#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseUInt32FixedPoint), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("", 10), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("", 10), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("123", 10), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("123", 10), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(0U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(0U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_no_separator_until_end_of_message)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("123", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt32FixedPoint(MAKE_CONTEXT("123", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_overflow)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345678901,", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345678901,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt32FixedPoint(MAKE_CONTEXT("100000000.99,", 0), 0, 2, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(13U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt32FixedPoint(MAKE_CONTEXT("999999999.9,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt32FixedPoint(MAKE_CONTEXT(".9999999999,", 0), 0, 10, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_no_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("abc,", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("abc.b,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_non_digit_characters_after_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("12c,", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("12c.b,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("123.b,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("123.1b,", 0), 0, 2, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_non_digit_characters_inbetween_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("1a3,", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("1a3.3,", 0), 0, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("13.3a3,", 0), 0, 3, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_double_dot_in_fixed_point_number)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.3.,", 0), 0, 2, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("13.3.,", 0), 0, 2, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("13.3.3,", 0), 0, 2, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt32FixedPoint(MAKE_CONTEXT("13.33.,", 0), 0, 3, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_fewer_than_minimal_number_of_whole_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1,", 0), 1, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa.1,", 2), 1, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("2.1,", 0), 2, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa2.1,", 2), 2, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("2,", 0), 2, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa2,", 2), 2, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("22,", 0), 3, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa22,", 2), 3, 1, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_empty_number)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_EMPTY_VALUE, ::parseUInt32FixedPoint(MAKE_CONTEXT(",", 0), 0, 0, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(1U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_EMPTY_VALUE, ::parseUInt32FixedPoint(MAKE_CONTEXT(",", 0), 0, 5, &result));
            Assert::AreEqual(0U, result);
            Assert::AreEqual(1U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_proccess_up_to_10_whole_digits_given_that_no_fractional_digits_is_expected)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1,", 0), 0, 0, &result));
            Assert::AreEqual(1U, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12,", 0), 0, 0, &result));
            Assert::AreEqual(12U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123,", 0), 0, 0, &result));
            Assert::AreEqual(123U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1234,", 0), 0, 0, &result));
            Assert::AreEqual(1234U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345,", 0), 0, 0, &result));
            Assert::AreEqual(12345U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123456,", 0), 0, 0, &result));
            Assert::AreEqual(123456U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123456,", 0), 0, 0, &result));
            Assert::AreEqual(123456U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1234567,", 0), 0, 0, &result));
            Assert::AreEqual(1234567U, result);
            Assert::AreEqual(8U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345678,", 0), 0, 0, &result));
            Assert::AreEqual(12345678U, result);
            Assert::AreEqual(9U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123456789,", 0), 0, 0, &result));
            Assert::AreEqual(123456789U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1234567890,", 0), 0, 0, &result));
            Assert::AreEqual(1234567890U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_process_up_to_max_number_of_digits_which_fit_into_uint32_give_number_of_fractional_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.2,", 0), 0, 1, &result));
            Assert::AreEqual(12U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12.3,", 0), 0, 1, &result));
            Assert::AreEqual(123U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123.4,", 0), 0, 1, &result));
            Assert::AreEqual(1234U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1234.5,", 0), 0, 1, &result));
            Assert::AreEqual(12345U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345.6,", 0), 0, 1, &result));
            Assert::AreEqual(123456U, result);
            Assert::AreEqual(8U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123456.7,", 0), 0, 1, &result));
            Assert::AreEqual(1234567U, result);
            Assert::AreEqual(9U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1234567.8,", 0), 0, 1, &result));
            Assert::AreEqual(12345678U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12345678.9,", 0), 0, 1, &result));
            Assert::AreEqual(123456789U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123456789.1,", 0), 0, 1, &result));
            Assert::AreEqual(1234567891U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_no_whole_digits_only_fractional)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1,", 0), 0, 1, &result));
            Assert::AreEqual(1U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12,", 0), 0, 2, &result));
            Assert::AreEqual(12U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123,", 0), 0, 3, &result));
            Assert::AreEqual(123U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234,", 0), 0, 4, &result));
            Assert::AreEqual(1234U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12345,", 0), 0, 5, &result));
            Assert::AreEqual(12345U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123456,", 0), 0, 6, &result));
            Assert::AreEqual(123456U, result);
            Assert::AreEqual(8U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567,", 0), 0, 7, &result));
            Assert::AreEqual(1234567U, result);
            Assert::AreEqual(9U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12345678,", 0), 0, 8, &result));
            Assert::AreEqual(12345678U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123456789,", 0), 0, 9, &result));
            Assert::AreEqual(123456789U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 10, &result));
            Assert::AreEqual(1234567891U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_ignore_extra_fractional_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 1, &result));
            Assert::AreEqual(1U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 2, &result));
            Assert::AreEqual(12U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 3, &result));
            Assert::AreEqual(123U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 4, &result));
            Assert::AreEqual(1234U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 5, &result));
            Assert::AreEqual(12345U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 6, &result));
            Assert::AreEqual(123456U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 7, &result));
            Assert::AreEqual(1234567U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 8, &result));
            Assert::AreEqual(12345678U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 9, &result));
            Assert::AreEqual(123456789U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 10, &result));
            Assert::AreEqual(1234567891U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_properly_handle_leading_whole_zeroes)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("0000000001,", 0), 0, 0, &result));
            Assert::AreEqual(1U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_properly_handle_trailing_fractional_zeroes)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1200000000,", 0), 0, 10, &result));
            Assert::AreEqual(1200000000U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_return_properly_formatted_number_if_there_are_no_fractional_digits)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1,", 0), 0, 9, &result));
            Assert::AreEqual(1000000000U, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123,", 0), 0, 1, &result));
            Assert::AreEqual(1230U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_return_properly_formatted_number_if_some_fractional_digits_are_missing_no_whole_numbers)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1,", 0), 0, 10, &result));
            Assert::AreEqual(1000000000U, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12,", 0), 0, 10, &result));
            Assert::AreEqual(1200000000U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123,", 0), 0, 10, &result));
            Assert::AreEqual(1230000000U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234,", 0), 0, 10, &result));
            Assert::AreEqual(1234000000U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12345,", 0), 0, 10, &result));
            Assert::AreEqual(1234500000U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123456,", 0), 0, 10, &result));
            Assert::AreEqual(1234560000U, result);
            Assert::AreEqual(8U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567,", 0), 0, 10, &result));
            Assert::AreEqual(1234567000U, result);
            Assert::AreEqual(9U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".12345678,", 0), 0, 10, &result));
            Assert::AreEqual(1234567800U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".123456789,", 0), 0, 10, &result));
            Assert::AreEqual(1234567890U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT(".1234567891,", 0), 0, 10, &result));
            Assert::AreEqual(1234567891U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_return_properly_formatted_number_if_some_fractional_digits_are_missing_with_whole_numbers)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1,", 0), 0, 9, &result));
            Assert::AreEqual(1100000000U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.12,", 0), 0, 9, &result));
            Assert::AreEqual(1120000000U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.123,", 0), 0, 9, &result));
            Assert::AreEqual(1123000000U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1234,", 0), 0, 9, &result));
            Assert::AreEqual(1123400000U, result);
            Assert::AreEqual(7U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.12345,", 0), 0, 9, &result));
            Assert::AreEqual(1123450000U, result);
            Assert::AreEqual(8U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.123456,", 0), 0, 9, &result));
            Assert::AreEqual(1123456000U, result);
            Assert::AreEqual(9U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1234567,", 0), 0, 9, &result));
            Assert::AreEqual(1123456700U, result);
            Assert::AreEqual(10U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.12345678,", 0), 0, 9, &result));
            Assert::AreEqual(1123456780U, result);
            Assert::AreEqual(11U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.123456789,", 0), 0, 9, &result));
            Assert::AreEqual(1123456789U, result);
            Assert::AreEqual(12U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_ignore_garbage_after_we_collected_required_number_of_characters)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1a,", 0), 0, 1, &result));
            Assert::AreEqual(11U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1.,", 0), 0, 1, &result));
            Assert::AreEqual(11U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.12d,", 0), 0, 2, &result));
            Assert::AreEqual(112U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_minimal_number_of_whole_digits_is_met)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("1.1,", 0), 1, 1, &result));
            Assert::AreEqual(11U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("12.1,", 0), 2, 1, &result));
            Assert::AreEqual(121U, result);
            Assert::AreEqual(5U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("123,", 0), 3, 1, &result));
            Assert::AreEqual(1230U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_work_if_starting_not_from_zero)
        {
            uint32_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa1.1,", 2), 1, 9, &result));
            Assert::AreEqual(1100000000U, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt32FixedPoint(MAKE_CONTEXT("aa1,", 2), 1, 9, &result));
            Assert::AreEqual(1000000000U, result);
            Assert::AreEqual(4U, context.tokenStartIdx);
        }
    };
};
