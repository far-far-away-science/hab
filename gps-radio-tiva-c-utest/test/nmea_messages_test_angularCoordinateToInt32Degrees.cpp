#include "..\stdafx.h"

#include "nmea_messages_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(nmea_messages_test_angularCoordinateToInt32Degrees), private NmeaTest
    {
        TEST_METHOD(Should_return_0_if_angular_coordinate_is_invalid)
        {
            AngularCoordinate coord;
            coord.degrees = 10;
            coord.minutes = 4731;
            coord.hemisphere = H_EAST;
            coord.isValid = false;
            Assert::AreEqual(0, angularCoordinateToInt32Degrees(coord));
        }

        TEST_METHOD(Should_return_fixed_number_if_coordinate_is_valid)
        {
            AngularCoordinate coord;
            coord.degrees = 120;
            coord.minutes = 47310000;
            coord.hemisphere = H_EAST;
            coord.isValid = true;
            Assert::AreEqual(120788496, angularCoordinateToInt32Degrees(coord));
        }
    };
}
