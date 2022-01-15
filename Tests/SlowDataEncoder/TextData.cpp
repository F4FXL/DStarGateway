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

#include "../../SlowDataEncoder.h"
#include "../../DStarDefines.h"

namespace SlowDataEncoderTests
{
    class SlowDataEncoder_textData : public ::testing::Test {
    
    };

    TEST_F(SlowDataEncoder_textData, textDataCorrectlySet)
    {
        CSlowDataEncoder encoder;
        encoder.setTextData("ABCDEFG");
        
        for(unsigned int testCount = 0U; testCount < 2U; testCount++) {
            unsigned char buffer[6U];

            encoder.getTextData(buffer);
            encoder.getTextData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x0U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'A');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'B');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'C');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'D');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

            encoder.getTextData(buffer);
            encoder.getTextData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x1U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'F');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getTextData(buffer);
            encoder.getTextData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x2U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getTextData(buffer);
            encoder.getTextData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_TEXT | 0x3U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            // all the remaining data shall be filled with 'f'
            for(unsigned int i = 24; i < 60U; i+= 6U) {
                encoder.getTextData(buffer);
                encoder.getTextData(buffer + 3);
                EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
                EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
                EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
                EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
            }
        }
    }
}
