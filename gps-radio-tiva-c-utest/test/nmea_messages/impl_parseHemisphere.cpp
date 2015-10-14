#include "..\..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseHemisphere), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseHemisphere(MAKE_CONTEXT("", 10), &result));
            Assert::AreEqual(10U, context.tokenStartIdx); // moves start to end of message
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseHemisphere(MAKE_CONTEXT("123", 10), &result));
            Assert::AreEqual(10U, context.tokenStartIdx); // moves start to end of message
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseHemisphere(MAKE_CONTEXT("", 0), &result));
            Assert::AreEqual(0U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_no_separator_until_end_of_message)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseHemisphere(MAKE_CONTEXT("N", 0), &result));
            Assert::AreEqual(1U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_more_than_one_character_in_hemisphere)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseHemisphere(MAKE_CONTEXT("NN,", 0), &result));
            Assert::AreEqual(3U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_unknown_hemisphere)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseHemisphere(MAKE_CONTEXT("D,", 0), &result));
            Assert::AreEqual(2U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_empty_hemisphere)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_EMPTY_VALUE, ::parseHemisphere(MAKE_CONTEXT(",", 0), &result));
            Assert::AreEqual(H_UNKNOWN, result);
            Assert::AreEqual(1U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_known_hemisphere)
        {
            HEMISPHERE result;
            Assert::AreEqual(NPR_VALID, ::parseHemisphere(MAKE_CONTEXT("N,", 0), &result));
            Assert::AreEqual(H_NORTH, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            Assert::AreEqual(NPR_VALID, ::parseHemisphere(MAKE_CONTEXT("S,", 0), &result));
            Assert::AreEqual(H_SOUTH, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            Assert::AreEqual(NPR_VALID, ::parseHemisphere(MAKE_CONTEXT("E,", 0), &result));
            Assert::AreEqual(H_EAST, result);
            Assert::AreEqual(2U, context.tokenStartIdx);

            Assert::AreEqual(NPR_VALID, ::parseHemisphere(MAKE_CONTEXT("W,", 0), &result));
            Assert::AreEqual(H_WEST, result);
            Assert::AreEqual(2U, context.tokenStartIdx);
        }
    };
}
