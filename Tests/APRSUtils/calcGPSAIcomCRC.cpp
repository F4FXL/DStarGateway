/*
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include "APRSUtils.h"

namespace APRStoDPRSTests
{
    class APRSUtils_calcGPSAIcomCRC : public ::testing::Test { 
    };

    TEST_F(APRSUtils_calcGPSAIcomCRC, withCRCHeader)
    {
        auto crc = CAPRSUtils::calcGPSAIcomCRC("$$CRC6F5E,ABCDEF");

        EXPECT_EQ(crc, 0x6f5e) << "CRC shall be valid";
    }

    TEST_F(APRSUtils_calcGPSAIcomCRC, withoutCRCHeader)
    {
        auto crc = CAPRSUtils::calcGPSAIcomCRC("ABCDEF");

        EXPECT_EQ(crc, 0x6f5e) << "CRC shall be valid";
    }
}