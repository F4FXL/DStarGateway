/*
 *   Copyright (C) 2021-2023 by Geoffrey Merck F4FXL / KC3FRA
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

#include "HeardData.h"

namespace HeardDataTests
{
    class HeardData_setIcomRepeaterData: public ::testing::Test {
    
    };

    TEST_F(HeardData_setIcomRepeaterData, setCorrectValues)
    {
        CHeardData hdata;

        in_addr addr;
        addr.s_addr=123;

        std::string data("0123456789abcdefghijklmnopqrstuvwxyz");

        hdata.setIcomRepeaterData((const unsigned char*)data.c_str(), data.length(), addr, 456U);

        EXPECT_STREQ(hdata.getUser().c_str(),     "abcdefgh") << "User not correctly set";
        EXPECT_STREQ(hdata.getRepeater().c_str(), "ijklmnop") << "Repeater not correctly set";
        EXPECT_EQ(hdata.getAddress().s_addr, 123) << "Addr not correctly set";
        EXPECT_EQ(hdata.getPort(), 456) << "Port not correctly set";
    }
}