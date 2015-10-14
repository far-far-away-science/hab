#include "..\..\stdafx.h"

#include "aprs_board_test.h"

namespace nmea_messages_test
{
    // one is encoded as no change in bit
    // zero is encoded as change in bit from 1 to 0 or from 0 to 1

    TEST_CLASS(aprs_board_test_encodeAndAppendBits)
    {
        TEST_METHOD_INITIALIZE(SetUp)
        {
            memset(&encodingData, 0, sizeof(EncodingData));
            memset(messageData, 0, sizeof(uint8_t) * MESSAGE_DATA_SIZE);
            memset(bitstreamBuffer, 0, sizeof(uint8_t) * BUFFER_DATA_SIZE);
        }

        TEST_METHOD(Should_encode_ones_as_no_change_in_bit_and_zeroes_like_change_from_one_to_zero_or_from_zero_to_one)
        {
            messageData[0] = 0b1111'0000;
            messageData[1] = 0b0101'0101;
            messageData[2] = 0b0000'0000;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 3, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 3, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 0, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b0000'0101, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b0110'0110, bitstreamBuffer[1]);
            Assert::AreEqual((uint8_t) 0b0101'0101, bitstreamBuffer[2]);
        }

        TEST_METHOD(Should_perform_stuffing_that_is_adding_0_after_five_ones_if_requested)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1111'1111;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_PERFORM_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 3, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b1110'0000, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b0000'0111, bitstreamBuffer[1]);
            Assert::AreEqual((uint8_t) 0b0000'0110, bitstreamBuffer[2]);
        }

        TEST_METHOD(While_performing_stuffing_0_bit_should_reset_count_of_ones)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1101'1110;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_PERFORM_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 1, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b1110'0000, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b1100'0001, bitstreamBuffer[1]);
        }

        TEST_METHOD(Should_not_perform_stuffing_if_not_requested)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1111'1111;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 0, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b0000'0000, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b0000'0000, bitstreamBuffer[1]);
        }

        TEST_METHOD(Should_remember_last_encoded_bit_if_its_zero)
        {
            messageData[0] = 0b1111'1111;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 1, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint8_t) 0, encodingData.lastBit);
        }

        TEST_METHOD(Should_remember_last_encoded_bit_if_its_one)
        {
            messageData[0] = 0b0111'1111;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 1, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint8_t) 1, encodingData.lastBit);
        }

        TEST_METHOD(Should_update_fcs_if_requested_but_no_stuffing)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 0xF590, encodingData.fcs);
        }

        TEST_METHOD(Should_update_fcs_if_requested_with_stuffing_but_stuffing_bits_should_not_affect_fcs)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO));

            const uint16_t noStuffingFcs = encodingData.fcs;

            memset(&encodingData, 0, sizeof(EncodingData));

            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual(noStuffingFcs, encodingData.fcs);
        }

        TEST_METHOD(Should_not_update_fcs_if_not_requested)
        {
            messageData[0] = 0b1111'1111;
            messageData[1] = 0b1010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t) 0x0, encodingData.fcs);
        }

        TEST_METHOD(Should_shift_all_bytes_one_bit_to_the_left_if_requested_without_fcs_or_stuffing)
        {
            messageData[0] = 0b0111'1111;
            messageData[1] = 0b0010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT));
            Assert::AreEqual((uint16_t) 2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 0, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b1111'1111, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b0110'0110, bitstreamBuffer[1]);
            Assert::AreEqual((uint16_t) 0x0, encodingData.fcs);
        }

        TEST_METHOD(Should_shift_all_bytes_one_bit_to_the_left_if_requested_should_affect_fcs_and_stuffing_if_they_were_requested)
        {
            messageData[0] = 0b0111'1111;
            messageData[1] = 0b0010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT));
            Assert::AreEqual((uint16_t) 2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t) 1, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t) 0b0011'1111, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t) 0b0011'0010, bitstreamBuffer[1]);
            Assert::AreEqual((uint8_t) 0b0000'0001, bitstreamBuffer[2]);
            Assert::AreEqual((uint16_t) 0xF2B9, encodingData.fcs);
        }

        TEST_METHOD(Should_not_shift_all_bytes_one_bit_to_the_left_if_not_requested)
        {
            messageData[0] = 0b0111'1111;
            messageData[1] = 0b0010'1010;
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 2, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::AreEqual((uint16_t)2, encodingData.bitstreamSize.bitstreamCharIdx);
            Assert::AreEqual((uint8_t)0, encodingData.bitstreamSize.bitstreamCharBitIdx);
            Assert::AreEqual((uint8_t)0b1000'0000, bitstreamBuffer[0]);
            Assert::AreEqual((uint8_t)0b0100'1100, bitstreamBuffer[1]);
        }

        TEST_METHOD(Should_return_false_in_case_of_buffer_overflow)
        {
            encodingData.bitstreamSize.bitstreamCharBitIdx = 7;
            Assert::IsFalse(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 10, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            encodingData.bitstreamSize.bitstreamCharIdx = 1;
            encodingData.bitstreamSize.bitstreamCharBitIdx = 0;
            Assert::IsFalse(encodeAndAppendBits(bitstreamBuffer, 10, &encodingData, messageData, 10, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        TEST_METHOD(Should_ignore_empty_message)
        {
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, BUFFER_DATA_SIZE, &encodingData, messageData, 0, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
            Assert::IsTrue(encodeAndAppendBits(bitstreamBuffer, 0, &encodingData, messageData, 0, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        TEST_METHOD(Should_return_false_if_message_is_not_empty_but_data_is_nullptr)
        {
            Assert::IsFalse(encodeAndAppendBits(bitstreamBuffer, BUFFER_DATA_SIZE, &encodingData, nullptr, MESSAGE_DATA_SIZE, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        TEST_METHOD(Should_return_false_if_encoding_data_is_nullptr)
        {
            Assert::IsFalse(encodeAndAppendBits(bitstreamBuffer, BUFFER_DATA_SIZE, nullptr, messageData, MESSAGE_DATA_SIZE, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        TEST_METHOD(Should_return_false_if_buffer_size_is_0)
        {
            Assert::IsFalse(encodeAndAppendBits(bitstreamBuffer, 0, &encodingData, messageData, MESSAGE_DATA_SIZE, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        TEST_METHOD(Should_return_false_if_buffer_is_nullptr)
        {
            Assert::IsFalse(encodeAndAppendBits(nullptr, 10, &encodingData, messageData, MESSAGE_DATA_SIZE, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO));
        }

        static const uint16_t MESSAGE_DATA_SIZE = 65535 / 2;
        static const uint16_t BUFFER_DATA_SIZE = 65535;

        EncodingData encodingData;
        uint8_t messageData[MESSAGE_DATA_SIZE];
        uint8_t bitstreamBuffer[BUFFER_DATA_SIZE];
    };
}
