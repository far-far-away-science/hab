#include "..\..\stdafx.h"

#include "aprs_board_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(aprs_board_test_createPacketPayload)
    {
        TEST_METHOD_INITIALIZE(SetUp)
        {
            gpsData.isValid = true;

            gpsData.gpggaData.utcTime.isValid = true;
            gpsData.gpggaData.utcTime.hours = 12;
            gpsData.gpggaData.utcTime.minutes = 17;
            gpsData.gpggaData.utcTime.seconds = 3344;

            gpsData.gpggaData.latitude.isValid = true;
            gpsData.gpggaData.latitude.hemisphere = H_NORTH;
            gpsData.gpggaData.latitude.degrees = 10;
            gpsData.gpggaData.latitude.minutes = 17224400;

            gpsData.gpggaData.longitude.isValid = true;
            gpsData.gpggaData.longitude.hemisphere = H_EAST;
            gpsData.gpggaData.longitude.degrees = 173;
            gpsData.gpggaData.longitude.minutes = 49246400;

            gpsData.gpggaData.fixType = GPSFT_GPS;
            gpsData.gpggaData.numberOfSattelitesInUse = 14;
            gpsData.gpggaData.altitudeMslMeters = 1900;

            gpsData.gpvtgData.speedKph = 130;
            gpsData.gpvtgData.trueCourseDegrees = 180;

            telemetry.cpuTemperature = 1700;
            telemetry.voltage = 900;

            gpsDataSource = GPS_ID_VENUS;

            memset(buffer, 0, BUFFER_SIZE);
        }

        void AssertAreEqual(const char* pExpectedBuffer, uint16_t messageIdx = 11)
        {
            const uint8_t expectedSize = (uint8_t) strlen(pExpectedBuffer);

            Assert::AreEqual(expectedSize, createPacketPayload(gpsDataSource, &gpsData, &telemetry, messageIdx, buffer, BUFFER_SIZE));
            Assert::AreEqual(0, memcmp(pExpectedBuffer, buffer, expectedSize));
        }

        TEST_METHOD(Should_format_fully_filled_message)
        {
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_format_message_without_time)
        {
            gpsData.gpggaData.utcTime.isValid = false;

            AssertAreEqual("!1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_format_message_without_latitude)
        {
            gpsData.gpggaData.latitude.isValid = false;

            AssertAreEqual("T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_format_message_without_longitude)
        {
            gpsData.gpggaData.longitude.isValid = false;

            AssertAreEqual("T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_format_message_without_latitude_and_longitude)
        {
            gpsData.gpggaData.latitude.isValid = false;
            gpsData.gpggaData.longitude.isValid = false;

            AssertAreEqual("T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_return_0_if_there_is_not_enough_space_for_string_with_longitude_and_latitude_and_time)
        {
            const char expectedBuffer[] = "@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190";
            const uint8_t expectedSize = (uint8_t) strlen(expectedBuffer);

            for (uint8_t i = 0; i < expectedSize; ++i)
            {
                Assert::AreEqual((uint8_t) 0, createPacketPayload(gpsDataSource, &gpsData, &telemetry, 11, buffer, i));
            }
        }

        TEST_METHOD(Should_return_0_if_there_is_not_enough_space_for_string_with_longitude_and_latitude_but_without_time)
        {
            gpsData.gpggaData.utcTime.isValid = false;

            const char expectedBuffer[] = "!1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190";
            const uint8_t expectedSize = (uint8_t)strlen(expectedBuffer);

            for (uint8_t i = 0; i < expectedSize; ++i)
            {
                Assert::AreEqual((uint8_t) 0, createPacketPayload(gpsDataSource, &gpsData, &telemetry, 11, buffer, i));
            }
        }

        TEST_METHOD(Should_return_0_if_there_is_not_enough_space_for_string_without_longitude_and_latitude)
        {
            gpsData.gpggaData.latitude.isValid = false;
            gpsData.gpggaData.longitude.isValid = false;

            const char expectedBuffer[] = "T#011,001,170,090,000,000,00000000 a=00190";
            const uint8_t expectedSize = (uint8_t)strlen(expectedBuffer);

            for (uint8_t i = 0; i < expectedSize; ++i)
            {
                Assert::AreEqual((uint8_t) 0, createPacketPayload(gpsDataSource, &gpsData, &telemetry, 11, buffer, i));
            }
        }

        TEST_METHOD(Should_properly_prefix_leading_zeroes_for_time)
        {
            gpsData.gpggaData.utcTime.hours = 0;
            AssertAreEqual("@001733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.hours = 1;
            AssertAreEqual("@011733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.hours = 13;
            AssertAreEqual("@131733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.utcTime.minutes = 0;
            AssertAreEqual("@130033z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.minutes = 7;
            AssertAreEqual("@130733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.minutes = 27;
            AssertAreEqual("@132733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.utcTime.seconds = 12;
            AssertAreEqual("@132700z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.seconds = 112;
            AssertAreEqual("@132701z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.utcTime.seconds = 3112;
            AssertAreEqual("@132731z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_latitude)
        {
            gpsData.gpggaData.latitude.degrees = 0;
            AssertAreEqual("@121733z0017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.degrees = 1;
            AssertAreEqual("@121733z0117.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.degrees = 27;
            AssertAreEqual("@121733z2717.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.latitude.minutes = 224400;
            AssertAreEqual("@121733z2700.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.minutes = 1224400;
            AssertAreEqual("@121733z2701.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.minutes = 18224400;
            AssertAreEqual("@121733z2718.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.latitude.minutes = 18000000;
            AssertAreEqual("@121733z2718.00N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.minutes = 18030000;
            AssertAreEqual("@121733z2718.03N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.latitude.minutes = 18430000;
            AssertAreEqual("@121733z2718.43N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.latitude.minutes = 18400000;
            AssertAreEqual("@121733z2718.40N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_longitude)
        {
            gpsData.gpggaData.longitude.degrees = 0;
            AssertAreEqual("@121733z1017.22N/00049.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.degrees = 1;
            AssertAreEqual("@121733z1017.22N/00149.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.degrees = 13;
            AssertAreEqual("@121733z1017.22N/01349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.degrees = 138;
            AssertAreEqual("@121733z1017.22N/13849.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.longitude.minutes = 246400;
            AssertAreEqual("@121733z1017.22N/13800.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.minutes = 1246400;
            AssertAreEqual("@121733z1017.22N/13801.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.minutes = 17246400;
            AssertAreEqual("@121733z1017.22N/13817.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.longitude.minutes = 17000000;
            AssertAreEqual("@121733z1017.22N/13817.00E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.minutes = 17010000;
            AssertAreEqual("@121733z1017.22N/13817.01E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsData.gpggaData.longitude.minutes = 17230000;
            AssertAreEqual("@121733z1017.22N/13817.23E>018/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpggaData.longitude.minutes = 17200000;
            AssertAreEqual("@121733z1017.22N/13817.20E>018/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_true_course)
        {
            gpsData.gpvtgData.trueCourseDegrees = 0;
            AssertAreEqual("@121733z1017.22N/17349.24E>000/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.trueCourseDegrees = 1;
            AssertAreEqual("@121733z1017.22N/17349.24E>000/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.trueCourseDegrees = 10;
            AssertAreEqual("@121733z1017.22N/17349.24E>001/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.trueCourseDegrees = 310;
            AssertAreEqual("@121733z1017.22N/17349.24E>031/013T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.trueCourseDegrees = 4310;
            AssertAreEqual("@121733z1017.22N/17349.24E>431/013T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_speed)
        {
            gpsData.gpvtgData.speedKph = 0;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/000T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.speedKph = 1;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/000T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.speedKph = 21;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/002T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.speedKph = 321;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/032T#011,001,170,090,000,000,00000000 a=00190");

            gpsData.gpvtgData.speedKph = 4321;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/432T#011,001,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_message_idx)
        {
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#000,001,170,090,000,000,00000000 a=00190", 0);
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#001,001,170,090,000,000,00000000 a=00190", 1);
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#032,001,170,090,000,000,00000000 a=00190", 32);
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#473,001,170,090,000,000,00000000 a=00190", 473);
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_gps_data_source)
        {
            gpsDataSource = GPS_ID_VENUS;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00190");
            gpsDataSource = GPS_ID_COPERNICUS;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,002,170,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_cpu_temperature)
        {
            telemetry.cpuTemperature = 0;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,000,090,000,000,00000000 a=00190");
            telemetry.cpuTemperature = 1;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,000,090,000,000,00000000 a=00190");
            telemetry.cpuTemperature = 10;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,001,090,000,000,00000000 a=00190");
            telemetry.cpuTemperature = 340;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,034,090,000,000,00000000 a=00190");
            telemetry.cpuTemperature = 3540;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,354,090,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_voltage)
        {
            telemetry.voltage = 0;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,000,000,000,00000000 a=00190");
            telemetry.voltage = 1;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,000,000,000,00000000 a=00190");
            telemetry.voltage = 43;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,004,000,000,00000000 a=00190");
            telemetry.voltage = 143;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,014,000,000,00000000 a=00190");
            telemetry.voltage = 5143;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,514,000,000,00000000 a=00190");
        }

        TEST_METHOD(Should_properly_add_leading_zeroes_for_altitude)
        {
            gpsData.gpggaData.altitudeMslMeters = 0;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00000");
            gpsData.gpggaData.altitudeMslMeters = 9;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00000");
            gpsData.gpggaData.altitudeMslMeters = 19;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00001");
            gpsData.gpggaData.altitudeMslMeters = 819;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00081");
            gpsData.gpggaData.altitudeMslMeters = 6819;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=00681");
            gpsData.gpggaData.altitudeMslMeters = 46819;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=04681");
            gpsData.gpggaData.altitudeMslMeters = 246819;
            AssertAreEqual("@121733z1017.22N/17349.24E>018/013T#011,001,170,090,000,000,00000000 a=24681");
        }

        static const uint8_t BUFFER_SIZE = 255;

        GpsData gpsData;
        Telemetry telemetry;
        GpsDataSource gpsDataSource;
        uint8_t buffer[BUFFER_SIZE];
    };
}
