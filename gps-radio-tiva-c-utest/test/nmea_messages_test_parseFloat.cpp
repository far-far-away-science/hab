#include "..\stdafx.h"

extern "C"
{
    #include <nmea_messages.h>
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace nmea_messages_test
{
    TEST_CLASS(ParseFloatTest)
    {
        Message* MAKE_MESSAGE(const char* messageText)
        {
            strcpy_s((char*)message.message, UART_MESSAGE_MAX_LEN, messageText);
            message.size = (uint8_t)strlen(messageText);
            return &message;
        }

        private:
            Message message;
    };
};
