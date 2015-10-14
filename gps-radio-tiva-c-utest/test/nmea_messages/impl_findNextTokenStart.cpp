#include "..\..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_findNextTokenStart), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::findNextTokenStart(MAKE_CONTEXT("", 10)));
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::findNextTokenStart(MAKE_CONTEXT("12.3,", 10)));
            Assert::AreEqual(10U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::findNextTokenStart(MAKE_CONTEXT("", 0)));
            Assert::AreEqual(0U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_no_separator_until_end_of_message)
        {
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::findNextTokenStart(MAKE_CONTEXT("12.3", 0)));
            Assert::AreEqual(4U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_skip_until_next_separator)
        {
            Assert::AreEqual(NPR_VALID, ::findNextTokenStart(MAKE_CONTEXT("12.3,", 0)));
            Assert::AreEqual(5U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_should_move_to_next_character_if_starting_at_separator)
        {
            Assert::AreEqual(NPR_VALID, ::findNextTokenStart(MAKE_CONTEXT(",", 0)));
            Assert::AreEqual(1U, context.tokenStartIdx);
        }
    };
};
