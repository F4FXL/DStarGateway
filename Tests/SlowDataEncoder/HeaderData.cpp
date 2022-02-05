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

#include "SlowDataEncoder.h"
#include "DStarDefines.h"

namespace SlowDataEncoderTests
{
    class SlowDataEncoder_headerData : public ::testing::Test {
    
    };

    TEST_F(SlowDataEncoder_headerData, headerDataCorrectlySet)
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

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, '1');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, '2');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '3');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'F');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, '5');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'Z');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'E');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'E');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'G');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'F');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '5');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'Z');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'E');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'E');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'B');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'C');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'Q');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'C');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'Q');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'C');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'Q');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, ' ');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'F');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, '4');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'F');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, 'X');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'L');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, ' ');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, ' ');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, ' ');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x5U);
            EXPECT_EQ(buffer[1] ^ SCRAMBLER_BYTE2, '5');
            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, '1');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, '0');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, '0');
            EXPECT_NE(buffer[5] ^ SCRAMBLER_BYTE3, 'f'); // we do not check the actual result of the CRC, we just make sure is inot 'f'

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
            EXPECT_EQ(buffer[0] ^ SCRAMBLER_BYTE1, SLOW_DATA_TYPE_HEADER | 0x1U);
            EXPECT_NE(buffer[1] ^ SCRAMBLER_BYTE2, 'f'); // done with crc check, remaining shall be filled with 'f'

            EXPECT_EQ(buffer[2] ^ SCRAMBLER_BYTE3, 'f');
            EXPECT_EQ(buffer[3] ^ SCRAMBLER_BYTE1, 'f');
            EXPECT_EQ(buffer[4] ^ SCRAMBLER_BYTE2, 'f');
            EXPECT_EQ(buffer[5] ^ SCRAMBLER_BYTE3, 'f');

            encoder.getHeaderData(buffer);
            encoder.getHeaderData(buffer + 3);
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
