#include "..\..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseUInt16FixedPoint), private NmeaTest
    {
        TEST_METHOD(Invalid_if_number_is_too_large_for_uint16)
        {
            uint16_t result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt16FixedPoint(MAKE_CONTEXT("99999,", 0), 0, 0, &result));
            Assert::AreEqual((uint16_t) 0, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_OVERFLOW, ::parseUInt16FixedPoint(MAKE_CONTEXT("999.99,", 0), 0, 2, &result));
            Assert::AreEqual((uint16_t) 0, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }

        TEST_METHOD(Invalid_if_underlying_parseUint32FixedPoint_fails)
        {
            uint16_t result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseUInt16FixedPoint(MAKE_CONTEXT("", 0), 0, 0, &result));
            Assert::AreEqual((uint16_t) 0, result);
            Assert::AreEqual(0U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseUInt16FixedPoint(MAKE_CONTEXT("999.9a,", 0), 0, 2, &result));
            Assert::AreEqual((uint16_t) 0, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }

        TEST_METHOD(Valid_sanity_tests)
        {
            uint16_t result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt16FixedPoint(MAKE_CONTEXT("65535,", 0), 0, 0, &result));
            Assert::AreEqual((uint16_t) 65535, result);
            Assert::AreEqual(6U, context.tokenStartIdx);

            result = 1;
            Assert::AreEqual(NPR_VALID, ::parseUInt16FixedPoint(MAKE_CONTEXT("655.35,", 0), 0, 2, &result));
            Assert::AreEqual((uint16_t) 65535, result);
            Assert::AreEqual(7U, context.tokenStartIdx);
        }
    };
};
