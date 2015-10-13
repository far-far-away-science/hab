#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseUInt8), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt8(MAKE_CONTEXT("", 10), 1, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt8(MAKE_CONTEXT("123", 10), 1, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt8(MAKE_CONTEXT("", 0), 1, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(0U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_overflow)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt8(MAKE_CONTEXT("256,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt8(MAKE_CONTEXT("100000,", 0), 6, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(6U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_no_digits)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("abcde,", 0), 5, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(5U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_non_digit_characters_after_digits)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("1a,", 0), 2, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(2U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_non_digit_characters_inbetween_digits)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("1a2,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_encountered_separator_with_limited_number_of_characters_should_also_move_start_index_to_start_of_next_token)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_SEPARATOR_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("1,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_SEPARATOR_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("12,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Should_parse_all_required_characters_even_if_invalid_characters_encountered)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("aab,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("12a,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt8(MAKE_CONTEXT("s2a,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 0, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt8(MAKE_CONTEXT("2,", 0), 1, &result));
            Assert::AreEqual((uint8_t) 2, result);
            Assert::AreEqual(1U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt8(MAKE_CONTEXT("12,", 0), 2, &result));
            Assert::AreEqual((uint8_t) 12, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt8(MAKE_CONTEXT("255,", 0), 3, &result));
            Assert::AreEqual((uint8_t) 255, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_starting_not_from_zero)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt8(MAKE_CONTEXT("aa2,", 2), 1, &result));
            Assert::AreEqual((uint8_t) 2, result);
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_be_okay_to_encounter_spearator_if_number_of_characters_is_unlimited)
        {
            uint8_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt8(MAKE_CONTEXT("123,", 0), NMEA_UNLIMITED_NUMBER_OF_CHARACTERS, &result));
            Assert::AreEqual((uint8_t) 123, result);
            Assert::AreEqual(4U, context.tokenStartIdx);
        }
    };
};
