#include "..\..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_test_parseGpggaMessageIfValid), private NmeaTest
    {
        TEST_METHOD(Valid_fully_filled_message)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"), &result);

            Assert::IsTrue(result.isValid);

            Assert::IsTrue(result.gpggaData.utcTime.isValid);
            Assert::AreEqual((uint8_t) 12, result.gpggaData.utcTime.hours);
            Assert::AreEqual((uint8_t) 35, result.gpggaData.utcTime.minutes);
            Assert::AreEqual((uint16_t) 1900, result.gpggaData.utcTime.seconds);

            Assert::IsTrue(result.gpggaData.latitude.isValid);
            Assert::AreEqual(H_NORTH, result.gpggaData.latitude.hemisphere);
            Assert::AreEqual((uint8_t) 48, result.gpggaData.latitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 7038000, result.gpggaData.latitude.minutes);

            Assert::IsTrue(result.gpggaData.longitude.isValid);
            Assert::AreEqual(H_EAST, result.gpggaData.longitude.hemisphere);
            Assert::AreEqual((uint8_t) 11, result.gpggaData.longitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 31000000, result.gpggaData.longitude.minutes);

            Assert::AreEqual(GPSFT_GPS, result.gpggaData.fixType);
            Assert::AreEqual((uint8_t) 8, result.gpggaData.numberOfSattelitesInUse);
            Assert::AreEqual((fixedPointW5F1_t) 5454, result.gpggaData.altitudeMslMeters);
        }

        TEST_METHOD(Valid_message_without_time)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"), &result);

            Assert::IsTrue(result.isValid);

            Assert::IsFalse(result.gpggaData.utcTime.isValid);

            Assert::IsTrue(result.gpggaData.latitude.isValid);
            Assert::AreEqual(H_NORTH, result.gpggaData.latitude.hemisphere);
            Assert::AreEqual((uint8_t) 48, result.gpggaData.latitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 7038000, result.gpggaData.latitude.minutes);

            Assert::IsTrue(result.gpggaData.longitude.isValid);
            Assert::AreEqual(H_EAST, result.gpggaData.longitude.hemisphere);
            Assert::AreEqual((uint8_t) 11, result.gpggaData.longitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 31000000, result.gpggaData.longitude.minutes);

            Assert::AreEqual(GPSFT_GPS, result.gpggaData.fixType);
            Assert::AreEqual((uint8_t) 8, result.gpggaData.numberOfSattelitesInUse);
            Assert::AreEqual((fixedPointW5F1_t) 5454, result.gpggaData.altitudeMslMeters);
        }

        TEST_METHOD(Valid_without_number_of_sattelites)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,01131.000,E,1,,0.9,54512.4,M,46.9,M,,*47"), &result);

            Assert::IsTrue(result.isValid);

            Assert::IsTrue(result.gpggaData.utcTime.isValid);
            Assert::AreEqual((uint8_t) 12, result.gpggaData.utcTime.hours);
            Assert::AreEqual((uint8_t) 35, result.gpggaData.utcTime.minutes);
            Assert::AreEqual((uint16_t) 1900, result.gpggaData.utcTime.seconds);

            Assert::IsTrue(result.gpggaData.latitude.isValid);
            Assert::AreEqual(H_NORTH, result.gpggaData.latitude.hemisphere);
            Assert::AreEqual((uint8_t) 48, result.gpggaData.latitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 7038000, result.gpggaData.latitude.minutes);

            Assert::IsTrue(result.gpggaData.longitude.isValid);
            Assert::AreEqual(H_EAST, result.gpggaData.longitude.hemisphere);
            Assert::AreEqual((uint8_t) 11, result.gpggaData.longitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 31000000, result.gpggaData.longitude.minutes);

            Assert::AreEqual(GPSFT_GPS, result.gpggaData.fixType);
            Assert::AreEqual((uint8_t) 0, result.gpggaData.numberOfSattelitesInUse);
            Assert::AreEqual((fixedPointW5F1_t) 545124, result.gpggaData.altitudeMslMeters);
        }

        TEST_METHOD(Valid_without_altitude)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,,M,46.9,M,,*47"), &result);

            Assert::IsTrue(result.isValid);

            Assert::IsTrue(result.gpggaData.utcTime.isValid);
            Assert::AreEqual((uint8_t) 12, result.gpggaData.utcTime.hours);
            Assert::AreEqual((uint8_t) 35, result.gpggaData.utcTime.minutes);
            Assert::AreEqual((uint16_t) 1900, result.gpggaData.utcTime.seconds);

            Assert::IsTrue(result.gpggaData.latitude.isValid);
            Assert::AreEqual(H_NORTH, result.gpggaData.latitude.hemisphere);
            Assert::AreEqual((uint8_t) 48, result.gpggaData.latitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 7038000, result.gpggaData.latitude.minutes);

            Assert::IsTrue(result.gpggaData.longitude.isValid);
            Assert::AreEqual(H_EAST, result.gpggaData.longitude.hemisphere);
            Assert::AreEqual((uint8_t) 11, result.gpggaData.longitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 31000000, result.gpggaData.longitude.minutes);

            Assert::AreEqual(GPSFT_GPS, result.gpggaData.fixType);
            Assert::AreEqual((uint8_t) 8, result.gpggaData.numberOfSattelitesInUse);
            Assert::AreEqual((fixedPointW5F1_t) 0, result.gpggaData.altitudeMslMeters);
        }

        TEST_METHOD(Valid_without_all_mandatory_fields)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,4807.038,N,01131.000,E,1,,,,,,,,*47"), &result);

            Assert::IsTrue(result.isValid);

            Assert::IsFalse(result.gpggaData.utcTime.isValid);

            Assert::IsTrue(result.gpggaData.latitude.isValid);
            Assert::AreEqual(H_NORTH, result.gpggaData.latitude.hemisphere);
            Assert::AreEqual((uint8_t) 48, result.gpggaData.latitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 7038000, result.gpggaData.latitude.minutes);

            Assert::IsTrue(result.gpggaData.longitude.isValid);
            Assert::AreEqual(H_EAST, result.gpggaData.longitude.hemisphere);
            Assert::AreEqual((uint8_t) 11, result.gpggaData.longitude.degrees);
            Assert::AreEqual((fixedPointW2F6_t) 31000000, result.gpggaData.longitude.minutes);

            Assert::AreEqual(GPSFT_GPS, result.gpggaData.fixType);
            Assert::AreEqual((uint8_t) 0, result.gpggaData.numberOfSattelitesInUse);
            Assert::AreEqual((fixedPointW5F1_t) 0, result.gpggaData.altitudeMslMeters);
        }

        TEST_METHOD(Invalid_empty_message)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,,,,,,,,,,,,,,*47"), &result);

            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_malformed_message)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA"), &result);

            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_message_without_latitude)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,,,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"), &result);

            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_message_without_longitude)
        {
            GpsData result = { 0 };
            parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,1,08,0.9,545.4,M,46.9,M,,*47"), &result);

            Assert::IsFalse(result.isValid);
        }

        TEST_METHOD(Invalid_message_with_invalid_gps_fix)
        {
            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,0,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,3,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,4,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,5,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,6,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }

            {
                GpsData result = { 0 };
                parseGpggaMessageIfValid(MAKE_MESSAGE("$GPGGA,123519,4807.038,N,,,8,08,0.9,545.4,M,46.9,M,,*47"), &result);
                Assert::IsFalse(result.isValid);
            }
        }

    };
}
