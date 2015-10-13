#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_test_parseGpvtgMessageIfValid), private NmeaTest
    {
        TEST_METHOD(Valid_fully_filled_message)
        {
            GpsData result = { 0 };
            parseGpvtgMessageIfValid(MAKE_MESSAGE("$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48"), &result);

            Assert::AreEqual((fixedPointW3F1_t) 547, result.gpvtgData.trueCourseDegrees);
            Assert::AreEqual((fixedPointW3F1_t) 102, result.gpvtgData.speedKph);
        }

        TEST_METHOD(Valid_true_course_missing)
        {
            GpsData result = { 0 };
            parseGpvtgMessageIfValid(MAKE_MESSAGE("$GPVTG,,T,034.4,M,005.5,N,010.2,K*48"), &result);

            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.trueCourseDegrees);
            Assert::AreEqual((fixedPointW3F1_t) 102, result.gpvtgData.speedKph);
        }

        TEST_METHOD(Valid_speed_missing)
        {
            GpsData result = { 0 };
            parseGpvtgMessageIfValid(MAKE_MESSAGE("$GPVTG,054.7,T,034.4,M,005.5,N,,K*48"), &result);

            Assert::AreEqual((fixedPointW3F1_t) 547, result.gpvtgData.trueCourseDegrees);
            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.speedKph);
        }

        TEST_METHOD(Valid_empty)
        {
            GpsData result = { 0 };
            parseGpvtgMessageIfValid(MAKE_MESSAGE("$GPVTG,,,,,,,,*48"), &result);

            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.trueCourseDegrees);
            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.speedKph);
        }

        TEST_METHOD(Valid_malformed)
        {
            GpsData result = { 0 };
            parseGpvtgMessageIfValid(MAKE_MESSAGE("$GPVTG"), &result);

            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.trueCourseDegrees);
            Assert::AreEqual((fixedPointW3F1_t) 0, result.gpvtgData.speedKph);
        }
    };
}
