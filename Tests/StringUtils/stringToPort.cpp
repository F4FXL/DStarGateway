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

#include "../../StringUtils.h"

namespace StringUtilsTests
{

    class StringUtils_stringToPort : public ::testing::Test {
    
    };

    TEST_F(StringUtils_stringToPort, EmptyStringReturn0)
    {
        unsigned int port = CStringUtils::stringToPort("");

        EXPECT_EQ(port, 0U);
    }

    TEST_F(StringUtils_stringToPort, SpaceStringReturn0)
    {
        unsigned int port = CStringUtils::stringToPort("  ");

        EXPECT_EQ(port, 0U);
    }

    TEST_F(StringUtils_stringToPort, NumberStringReturnsCorrectValue)
    {
        unsigned int port = CStringUtils::stringToPort("12345");

        EXPECT_EQ(port, 12345U);
    }

    TEST_F(StringUtils_stringToPort, NumberStringWithSpacesReturnsCorrectValue)
    {
        unsigned int port = CStringUtils::stringToPort("  12345  ");

        EXPECT_EQ(port, 12345U);
    }

    TEST_F(StringUtils_stringToPort, StringWithMixedAlphaAndNumbersreturns0)
    {
        unsigned int port = CStringUtils::stringToPort("123abc456");

        EXPECT_EQ(port, 0U);
    }

    TEST_F(StringUtils_stringToPort, TooLargeValueReturns0)
    {
        unsigned int port = CStringUtils::stringToPort("999999");

        EXPECT_EQ(port, 0U);
    }


    TEST_F(StringUtils_stringToPort, TestAllNumDigits)
    {
        EXPECT_EQ(CStringUtils::stringToPort("10"), 10U);
        EXPECT_EQ(CStringUtils::stringToPort("11"), 11U);
        EXPECT_EQ(CStringUtils::stringToPort("12"), 12U);
        EXPECT_EQ(CStringUtils::stringToPort("13"), 13U);
        EXPECT_EQ(CStringUtils::stringToPort("14"), 14U);
        EXPECT_EQ(CStringUtils::stringToPort("15"), 15U);
        EXPECT_EQ(CStringUtils::stringToPort("16"), 16U);
        EXPECT_EQ(CStringUtils::stringToPort("17"), 17U);
        EXPECT_EQ(CStringUtils::stringToPort("18"), 18U);
        EXPECT_EQ(CStringUtils::stringToPort("19"), 19U);
    }
}