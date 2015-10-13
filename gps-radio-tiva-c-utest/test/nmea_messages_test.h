#pragma once

extern "C"
{
    #include <nmea_messages_impl.h>
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework {
    template<> inline std::wstring ToString<NMEA_PARSING_RESULT_t>(const NMEA_PARSING_RESULT_t& t) { RETURN_WIDE_STRING(t); }
    template<> inline std::wstring ToString<HEMISPHERE_t>(const HEMISPHERE_t& t) { RETURN_WIDE_STRING(t); }
    template<> inline std::wstring ToString<GPS_FIX_TYPE_t>(const GPS_FIX_TYPE_t& t) { RETURN_WIDE_STRING(t); }
    template<> inline std::wstring ToString<uint16_t>(const uint16_t& t) { RETURN_WIDE_STRING(t); }
}}}

class NmeaTest
{
    protected:
        Message* MAKE_MESSAGE(const char* messageText)
        {
            strcpy_s((char*) message.message, UART_MESSAGE_MAX_LEN, messageText);
            message.size = (uint8_t) strlen(messageText);

            return &message;
        }

        NmeaParsingContext* MAKE_CONTEXT(const char* messageText, uint32_t tokenStartIdx)
        {
            context.pMessage = MAKE_MESSAGE(messageText);
            context.tokenStartIdx = tokenStartIdx;

            return &context;
        }

        Message message;
        NmeaParsingContext context;
};
