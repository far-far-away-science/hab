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
    template<> inline std::wstring ToString<uint16_t>(const uint16_t& t) { RETURN_WIDE_STRING(t); }
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
            Assert::IsTrue(result.utcTime.isValid);
            Assert::AreEqual((uint8_t) 6, result.utcTime.hours);
            Assert::AreEqual((uint8_t) 28, result.utcTime.minutes);
            Assert::AreEqual(1.835f, result.utcTime.seconds);
            Assert::IsTrue(result.latitude.isValid);
            Assert::AreEqual((uint16_t) 47, result.latitude.degrees);
            Assert::AreEqual(32.7089f, result.latitude.minutes);
            Assert::AreEqual(LATH_NORTH, result.latitudeHemisphere);
            Assert::IsTrue(result.longitude.isValid);
            Assert::AreEqual((uint16_t) 122, result.longitude.degrees);
            Assert::AreEqual(1.0455f, result.longitude.minutes);
            Assert::AreEqual(LONH_WEST, result.longitudeHemisphere);
            Assert::AreEqual(GPSFT_GPS, result.fixType);
            Assert::AreEqual(179.6f, result.altitudeMslMeters);
            Assert::AreEqual((uint8_t) 3, result.numberOfSattelitesInUse);
        }

        TEST_METHOD(ShouldParseAllFieldsOfMessageWithDgpsFixAndWithoutTimeAndAltitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4732.70072,N,12200.86047,W,2,00,,,,,,,*44"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsTrue(result.latitude.isValid);
            Assert::AreEqual((uint16_t)47, result.latitude.degrees);
            Assert::AreEqual(32.70072f, result.latitude.minutes);
            Assert::AreEqual(LATH_NORTH, result.latitudeHemisphere);
            Assert::IsTrue(result.longitude.isValid);
            Assert::AreEqual((uint16_t)122, result.longitude.degrees);
            Assert::AreEqual(00.86047f, result.longitude.minutes);
            Assert::AreEqual(LONH_WEST, result.longitudeHemisphere);
            Assert::AreEqual(GPSFT_DGPS, result.fixType);
            Assert::IsTrue(isnan(result.altitudeMslMeters));
            Assert::AreEqual((uint8_t) 0, result.numberOfSattelitesInUse);
        }

        TEST_METHOD(ShouldIgnoreNoLatitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,,N,12201.0455,W,1,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        TEST_METHOD(ShouldIgnoreNoLongitude)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,062801.835,4732.7089,N,,W,1,03,3.5,179.6,M,-18.1,M,,0000*64"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        TEST_METHOD(ShouldIgnoreAllFieldsOfEmptyMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,,,,,,,,,,,,,"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        TEST_METHOD(ShouldIgnoreMalformedMessage)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4732.70047,W,7,00,,,,,,,7,"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
            ::parseGpggaMessageIfValid(MAKE_MESSAGE("GA,,4732.70072,N"), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        TEST_METHOD(ShouldIgnoreEmptyString)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(MAKE_MESSAGE(""), &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        TEST_METHOD(ShouldIgnoreNullptrString)
        {
            GpsData result = { 0 };
            ::parseGpggaMessageIfValid(nullptr, &result);
            Assert::IsFalse(result.utcTime.isValid);
            Assert::IsFalse(result.latitude.isValid);
            Assert::IsFalse(result.longitude.isValid);
        }

        private:
            Message message;
    };
}
