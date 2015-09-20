#include "..\stdafx.h"

extern "C"
{
    #include <nmea_messages.h>
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework {
    template<> inline std::wstring ToString<LATITUDE_HEMISPHERE_t>(const LATITUDE_HEMISPHERE_t& t) { RETURN_WIDE_STRING(t); }
    template<> inline std::wstring ToString<LONGITUDE_HEMISPHERE_t>(const LONGITUDE_HEMISPHERE_t& t) { RETURN_WIDE_STRING(t); }
    template<> inline std::wstring ToString<GPS_FIX_TYPE_t>(const GPS_FIX_TYPE_t& t) { RETURN_WIDE_STRING(t); }
}}}

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

        TEST_METHOD(ShouldParseAllFieldsOfFullyFilledMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,1,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsTrue(result.isValid);
            Assert::IsTrue(result.utcTime.isValid);
            Assert::AreEqual((uint8_t) 6, result.utcTime.hours);
            Assert::AreEqual((uint8_t) 28, result.utcTime.minutes);
            Assert::AreEqual(1.835f, result.utcTime.seconds);
            Assert::AreEqual(47.5451483333f, result.latitudeDegrees);
            Assert::AreEqual(LATH_NORTH, result.latitudeHemisphere);
            Assert::AreEqual(122.017425f, result.longitudeDegrees);
            Assert::AreEqual(LONH_WEST, result.longitudeHemisphere);
            Assert::AreEqual(GPSFT_GPS, result.fixType);
            Assert::AreEqual(179.6f, result.altitudeMslMeters);
            Assert::AreEqual((uint8_t) 3, result.numberOfSattelitesInUse);
        }

        TEST_METHOD(ShouldParseAllFieldsOfMessageWithDgpsFixAndWithoutTimeAndAltitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4732.70072,N,12200.86047,W,2,00,,,,,,,*44"), &result);
            Assert::IsTrue(result.isValid);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::AreEqual(47.545012f, result.latitudeDegrees);
            Assert::AreEqual(LATH_NORTH, result.latitudeHemisphere);
            Assert::AreEqual(122.014341166f, result.longitudeDegrees);
            Assert::AreEqual(LONH_WEST, result.longitudeHemisphere);
            Assert::AreEqual(GPSFT_DGPS, result.fixType);
            Assert::IsTrue(isnan(result.altitudeMslMeters));
            Assert::AreEqual((uint8_t) 0, result.numberOfSattelitesInUse);
        }

        TEST_METHOD(ShouldParseManualInputMode)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,7,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsTrue(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreNoFixMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,0,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,3,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,4,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,5,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,6,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,12201.0455,W,8,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreNoLatitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,,N,12201.0455,W,1,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreNoLongitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,,W,1,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreAllFieldsOfEmptyMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,,,,,,,,,,,,,"), &result);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreMalformedMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4732.70047,W,7,00,,,,,,,7,"), &result);
            Assert::IsFalse(result.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("GA,,4732.70072,N"), &result);
            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(ShouldIgnoreEmpty)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE(""), &result);
            Assert::IsFalse(result.isValid);
        }

        private:
            Message message;
    };
}
