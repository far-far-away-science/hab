#include "..\..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_canUInt32Overflow), private NmeaTest
    {
        TEST_METHOD(Should_return_false_if_new_number_wont_overflow)
        {
            Assert::IsFalse(canUInt32Overflow(429496728, 0));
            Assert::IsFalse(canUInt32Overflow(429496728, 1));
            Assert::IsFalse(canUInt32Overflow(429496728, 2));
            Assert::IsFalse(canUInt32Overflow(429496728, 3));
            Assert::IsFalse(canUInt32Overflow(429496728, 4));
            Assert::IsFalse(canUInt32Overflow(429496728, 5));
            Assert::IsFalse(canUInt32Overflow(429496728, 6));
            Assert::IsFalse(canUInt32Overflow(429496728, 7));
            Assert::IsFalse(canUInt32Overflow(429496728, 8));
            Assert::IsFalse(canUInt32Overflow(429496728, 9));

            Assert::IsFalse(canUInt32Overflow(429496729, 0));
            Assert::IsFalse(canUInt32Overflow(429496729, 1));
            Assert::IsFalse(canUInt32Overflow(429496729, 2));
            Assert::IsFalse(canUInt32Overflow(429496729, 3));
            Assert::IsFalse(canUInt32Overflow(429496729, 4));
            Assert::IsFalse(canUInt32Overflow(429496729, 5));
        }

        TEST_METHOD(Should_return_true_if_new_number_overflows)
        {
            Assert::IsTrue(canUInt32Overflow(429496729, 6));
            Assert::IsTrue(canUInt32Overflow(429496729, 7));
            Assert::IsTrue(canUInt32Overflow(429496729, 8));
            Assert::IsTrue(canUInt32Overflow(429496729, 9));

            Assert::IsTrue(canUInt32Overflow(429496730, 0));
            Assert::IsTrue(canUInt32Overflow(429496730, 1));
            Assert::IsTrue(canUInt32Overflow(429496730, 2));
            Assert::IsTrue(canUInt32Overflow(429496730, 3));
            Assert::IsTrue(canUInt32Overflow(429496730, 4));
            Assert::IsTrue(canUInt32Overflow(429496730, 5));
            Assert::IsTrue(canUInt32Overflow(429496730, 6));
            Assert::IsTrue(canUInt32Overflow(429496730, 7));
            Assert::IsTrue(canUInt32Overflow(429496730, 8));
            Assert::IsTrue(canUInt32Overflow(429496730, 9));
        }
    };
};
