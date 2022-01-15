/*
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "../../SlowDataEncoder.h"
#include "../../DStarDefines.h"

namespace SlowDataEncoderTests
{
    class SlowDataEncoder_interleavedData : public ::testing::Test {
    
    };

    TEST_F(SlowDataEncoder_interleavedData, gpsAndHeaderData)
    {
                // Header is never interleaved, text and header are sent as two blocks
        CHeaderData header;
        unsigned char headerData[RADIO_HEADER_LENGTH_BYTES];
        ::memset(headerData, 'H', RADIO_HEADER_LENGTH_BYTES);
        header.setData(headerData, RADIO_HEADER_LENGTH_BYTES, false);

        //here we only test for correct interleaving
        CSlowDataEncoder encoder;
        encoder.setGPSData("GGGGGGGG"); // 8 times G
        encoder.setHeaderData(header);

        for(unsigned int i = 0U; i < 2U; i++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            // all the remaining data shall be filled with 'f' until next block
            for(unsigned int j = 12; j < 60U; j+= 6U) {
                encoder.getInterleavedData(buffer);
                encoder.getInterleavedData(buffer + 3);
                EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
                EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
            }

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_NE(buffer[5] ^ SCRAMBLER_BYTE3, 'f');// this is checksum byte, just make sure is is not 'f'

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x1U);
            EXPECT_NE(buffer[1] ^ SCRAMBLER_BYTE2, 'f');// this is checksum byte, just make sure is is not 'f'
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            //remaining data is only 'f'
            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, textAndHeaderData)
    {
        // Header is never interleaved, text and header are sent as two blocks
        CHeaderData header;
        unsigned char headerData[RADIO_HEADER_LENGTH_BYTES];
        ::memset(headerData, 'H', RADIO_HEADER_LENGTH_BYTES);
        header.setData(headerData, RADIO_HEADER_LENGTH_BYTES, false);

        //here we only test for correct interleaving
        CSlowDataEncoder encoder;
        encoder.setTextData("TTTTTT"); // 6 times T
        encoder.setHeaderData(header);

        auto dataLen = encoder.getInterleavedDataLength();
        EXPECT_EQ(dataLen, 120); //2* 60

        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x0U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'T');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'T');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'T');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x1U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            // all the remaining data shall be filled with 'f' until next block
            for(unsigned int j = 24; j < 60U; j+= 6U) {
                encoder.getInterleavedData(buffer);
                encoder.getInterleavedData(buffer + 3);
                EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
                EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
            }

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_NE(buffer[5] ^ SCRAMBLER_BYTE3, 'f');// this is checksum byte, just make sure is is not 'f'

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x1U);
            EXPECT_NE(buffer[1] ^ SCRAMBLER_BYTE2, 'f');// this is checksum byte, just make sure is is not 'f'
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            //remaining data is only 'f'
            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, gpsTextAndHeaderData)
    {
        CHeaderData header;
        unsigned char headerData[RADIO_HEADER_LENGTH_BYTES];
        ::memset(headerData, 'H', RADIO_HEADER_LENGTH_BYTES);
        header.setData(headerData, RADIO_HEADER_LENGTH_BYTES, false);

        //here we only test for correct interleaving
        CSlowDataEncoder encoder;
        encoder.setTextData("TTTTTT"); // 6 times T
        encoder.setGPSData("GGGGGGGGGGGGGGGGGGGGGG"); // 22 times G
        encoder.setHeaderData(header);

        auto dataLen = encoder.getInterleavedDataLength();

        EXPECT_EQ(dataLen, 120);// including data type bytes we need 54 (20 + 5 + 22 + 6) bytes, this shall be rounded up to next block size multiple, in this case 60

        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x0U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'T');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'T');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'T');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x1U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');


            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            // header is not interleaved, attached as one contiguous block, but it only starts at the nex block
            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'H');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'H');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'H');
            EXPECT_NE(buffer[5] ^ SCRAMBLER_BYTE3, 'f');// this is checksum byte, just make sure is is not 'f'

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x1U);
            EXPECT_NE(buffer[1] ^ SCRAMBLER_BYTE2, 'f');// this is checksum byte, just make sure is is not 'f'
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            //remaining data is only 'f'
            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, gpsAndTextData)
    {
        //here we only test for correct interleaving
        CSlowDataEncoder encoder;
        encoder.setTextData("TTTTTT"); // 6 times T
        encoder.setGPSData("GGGGGGGGGGGGGGGGGGGGGG"); // 22 times G

        auto dataLen = encoder.getInterleavedDataLength();

        EXPECT_EQ(dataLen, 60);// including data type bytes we need 54 (20 + 5 + 22 + 6) bytes, this shall be rounded up to next block size multiple, in this case 60

        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x0U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'T');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'T');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'T');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x1U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'T');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'G');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'G');


            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            //remaining shall only be 'f"
            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, onlyGPSData)
    {
        CSlowDataEncoder encoder;
        encoder.setGPSData("ABCDEFGHIJKLMN");

        auto dataLen = encoder.getInterleavedDataLength();
        EXPECT_EQ(dataLen, 60); 
        
        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'A');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'B');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'C');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'D');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'F');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'I');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'J');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x4U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'K');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'L');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'M');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'N');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            // all the remaining data shall be filled with 'f'
            for(unsigned int i = 18; i < 60U; i+= 6U) {
                encoder.getInterleavedData(buffer);
                encoder.getInterleavedData(buffer + 3);
                EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
                EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
            }
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, onlyTextData)
    {
        CSlowDataEncoder encoder;
        encoder.setTextData("ABCDEFG");

        auto dataLen = encoder.getInterleavedDataLength();
        EXPECT_EQ(dataLen, 60);
        
        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x0U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'A');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'B');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'C');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'D');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x1U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'F');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            // all the remaining data shall be filled with 'f'
            for(unsigned int i = 24; i < 60U; i+= 6U) {
                encoder.getInterleavedData(buffer);
                encoder.getInterleavedData(buffer + 3);
                EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
                EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
            }
        }
    }

    TEST_F(SlowDataEncoder_interleavedData, onlyHeaderData)
    {
        CHeaderData header;
        header.setFlags('1', '2', '3');
        header.setMyCall1("F4FXL");
        header.setMyCall2("5100");
        header.setYourCall("CQCQCQ");
        header.setRptCall1("F5ZEE  B");
        header.setRptCall2("F5ZEE  G");
        
        CSlowDataEncoder encoder;
        encoder.setHeaderData(header);
        
        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, '1');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, '2');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '3');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'F');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, '5');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'Z');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'E');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'E');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'F');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '5');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'Z');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'E');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'B');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'C');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'Q');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'C');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'Q');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'C');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'Q');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'F');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, '4');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'F');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'X');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'L');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, '5');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, '1');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '0');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, '0');
            EXPECT_NE(buffer[5] ^ SCRAMBLER_BYTE3, 'f'); // we do not check the actual result of the CRC, we just make sure is inot 'f'

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x1U);
            EXPECT_NE(buffer[1] ^ SCRAMBLER_BYTE2, 'f'); // done with crc check, remaining shall be filled with 'f'

            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            encoder.getInterleavedData(buffer);
            encoder.getInterleavedData(buffer + 3);
            // all the remaining data shall be filled with 'f'
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }
}
