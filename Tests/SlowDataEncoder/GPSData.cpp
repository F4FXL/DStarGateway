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
    class SlowDataEncoder_gpsData : public ::testing::Test {
    
    };

    TEST_F(SlowDataEncoder_gpsData, gpsDataCorrectlySet)
    {
        CSlowDataEncoder encoder;
        encoder.setGPSData("ABCDEFGHIJKLMN");

        unsigned char buffer[6U];

        encoder.getGPSData(buffer);
        encoder.getGPSData(buffer + 3);
        EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
        EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'A');
        EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'B');
        EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'C');
        EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'D');
        EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

        encoder.getGPSData(buffer);
        encoder.getGPSData(buffer + 3);
        EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x5U);
        EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'F');
        EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'G');
        EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'H');
        EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'I');
        EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'J');

        encoder.getGPSData(buffer);
        encoder.getGPSData(buffer + 3);
        EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_GPS | 0x4U);
        EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'K');
        EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'L');
        EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'M');
        EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'N');
        EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

        for(unsigned int i = 18; i < 60U; i+= 6U) {
            encoder.getGPSData(buffer);
            encoder.getGPSData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');
        }
    }
}