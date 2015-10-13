#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_impl_parseAngularCoordinate_latitude), private NmeaTest
    {
        TEST_METHOD(Invalid_start_after_zero_length)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseAngularCoordinate(MAKE_CONTEXT("", 10), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_start_after_non_zero_length)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseAngularCoordinate(MAKE_CONTEXT("abc", 10), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_start_at_0_and_zero_length)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseAngularCoordinate(MAKE_CONTEXT("", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(0U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_no_separator_until_end_of_message)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseAngularCoordinate(MAKE_CONTEXT("1212.23", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(7U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_missing_hemisphere_separator)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_END_OF_MESSAGE, ::parseAngularCoordinate(MAKE_CONTEXT("1212.23,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(8U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_only_one_number_in_degrees)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseAngularCoordinate(MAKE_CONTEXT("1,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(2U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_no_minute_digits)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseAngularCoordinate(MAKE_CONTEXT("12,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(3U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_only_one_digit_in_minutes)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_NOT_ENOUGH_DIGITS, ::parseAngularCoordinate(MAKE_CONTEXT("123,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(4U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_empty_angular_coordinate)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseAngularCoordinate(MAKE_CONTEXT(",N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(3U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_empty_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseAngularCoordinate(MAKE_CONTEXT("1212.23,,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(9U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_degrees_overflow)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_OVERFLOW, ::parseAngularCoordinate(MAKE_CONTEXT("9112.23,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_minutes_overflow)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_OVERFLOW, ::parseAngularCoordinate(MAKE_CONTEXT("1260.00,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_east_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseAngularCoordinate(MAKE_CONTEXT("1212.99,E,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_west_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_UNEXPECTED_CHARACTER_ENCOUNTERED, ::parseAngularCoordinate(MAKE_CONTEXT("1212.99,W,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Valid_empty_angular_coordinate_and_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_EMPTY_VALUE, ::parseAngularCoordinate(MAKE_CONTEXT(",,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(2U, context.tokenStartIdx);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Valid_normal_angular_coordinate)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.123456,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(14U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59123456, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_normal_angular_coordinate_without_fractional_minutes)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(7U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59000000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_normal_angular_coordinate_with_partial_fractional_minutes)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.9,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(9U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59900000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);

            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.91,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59910000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);

            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.912,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(11U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59912000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);

            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.9123,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(12U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59912300, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);

            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.91234,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(13U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59912340, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);

            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.912345,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(14U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59912345, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_normal_angular_coordinate_with_more_than_6_fractional_minutes)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.9123456,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(15U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59912345, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_normal_angular_coordinate_with_non_digit_characters_at_the_end)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1259.990000z,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(15U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59990000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_lots_of_zeroes)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("0000.00,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 0, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 0, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_max_values)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("9059.999999,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(14U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 90, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 59999999, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_north_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1212.99,N,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 12990000, result.minutes);
            Assert::AreEqual(H_NORTH, result.hemisphere);
        }

        TEST_METHOD(Valid_south_hemisphere)
        {
            AngularCoordinate result;
            result.isValid = true;
            Assert::AreEqual(NPR_VALID, ::parseAngularCoordinate(MAKE_CONTEXT("1212.99,S,", 0), ACR_LATITUDE, &result));
            Assert::AreEqual(10U, context.tokenStartIdx);
            Assert::IsTrue(result.isValid);
            Assert::AreEqual((uint8_t) 12, result.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 12990000, result.minutes);
            Assert::AreEqual(H_SOUTH, result.hemisphere);
        }
    };
}
