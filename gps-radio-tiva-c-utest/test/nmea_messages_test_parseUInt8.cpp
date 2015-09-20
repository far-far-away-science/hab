#include "..\stdafx.h"

extern "C"
{
    #include <nmea_messages.h>
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace nmea_messages_test
{
    TEST_CLASS(ParseUInt8Test)
    {
        Message* MAKE_MESSAGE(const char* messageText)
        {
            strcpy_s((char*) message.message, UART_MESSAGE_MAX_LEN, messageText);
            message.size = (uint8_t) strlen(messageText);
            return &message;
        }

        TEST_METHOD(ShouldReturn0IfEmptyString)
        {
            uint8_t result;
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("12"), 0, 0, &result));
        }

        TEST_METHOD(ShouldReturn0IfNullMessage)
        {
            uint8_t result;
            Assert::IsFalse(::parseUInt8(nullptr, 0, 2, &result));
        }

        TEST_METHOD(ShouldReturn0IfNullResult)
        {
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("12"), 0, 2, nullptr));
        }

        TEST_METHOD(ShouldReturn0IfNumberIsTooLarge)
        {
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("999"), 0, 3, nullptr));
        }

        TEST_METHOD(ShouldAbortOnWhiteSpace)
        {
            uint8_t result;
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE(" 10"), 0, 3, &result));
        }

        TEST_METHOD(ShouldAbortIfNumberIsTooLarge)
        {
            uint8_t result;
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("1101"), 0, 4, &result));
        }

        TEST_METHOD(ShouldAbortAtNonDigit)
        {
            uint8_t result;
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("12  10a"), 4, 7, &result));
            Assert::IsFalse(::parseUInt8(MAKE_MESSAGE("12  10 "), 4, 7, &result));
        }

        TEST_METHOD(ShouldParseNormalDigits)
        {
            uint8_t result;
            Assert::IsTrue(::parseUInt8(MAKE_MESSAGE("255"), 0, 3, &result));
            Assert::AreEqual((uint8_t) 255, result);
            Assert::IsTrue(::parseUInt8(MAKE_MESSAGE("055"), 0, 3, &result));
            Assert::AreEqual((uint8_t) 55, result);
            Assert::IsTrue(::parseUInt8(MAKE_MESSAGE("55"), 0, 2, &result));
            Assert::AreEqual((uint8_t) 55, result);
            Assert::IsTrue(::parseUInt8(MAKE_MESSAGE("5"), 0, 1, &result));
            Assert::AreEqual((uint8_t) 5, result);
            Assert::IsTrue(::parseUInt8(MAKE_MESSAGE("  55"), 2, 4, &result));
            Assert::AreEqual((uint8_t) 55, result);
        }

        private:
            Message message;
    };
};
