#include "..\..\stdafx.h"

#include "aprs_board_test.h"

namespace nmea_messages_test
{
    TEST_CLASS(aprs_board_test_advanceBitstreamBit)
    {
        TEST_METHOD(Should_advance_to_next_bit_if_current_bit_is_less_than_7)
        {
            BitstreamPos pos = { 0, 0 };
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 1, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 2, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 3, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 4, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 5, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 6, pos.bitstreamCharBitIdx);
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 7, pos.bitstreamCharBitIdx);
        }

        TEST_METHOD(Should_move_back_to_0_if_current_bit_is_7_or_more)
        {
            BitstreamPos pos = { 0, 7 };
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 0, pos.bitstreamCharBitIdx);
            pos.bitstreamCharBitIdx = 100;
            advanceBitstreamBit(&pos);
            Assert::AreEqual((uint8_t) 0, pos.bitstreamCharBitIdx);
        }
    };
}
