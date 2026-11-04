/*
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include "Utils.h"
#include "StringUtils.h"
#include "Log.h"

class Utils_swap_endian_be : public ::testing::Test {
 
};

// This test will fail on big endian systems....
TEST_F(Utils_swap_endian_be, SwapUINT32_be) {
    uint32_t test = 0x12345678U;

    uint32_t res = CUtils::swap_endian_be(test);

    EXPECT_EQ(res, 0x12345678U);
}


TEST_F(Utils_swap_endian_be, blabla) {
    // explicit casts keep the byte values identical on platforms where char is unsigned
    std::vector<char> charsToEscape = {(char)-17, (char)0, (char)17, (char)19, (char)(128-2), (char)-25, (char)26, (char)(128-3), (char)-1, (char)36, (char)13, (char)44};

    for(unsigned int i = 1U; i < charsToEscape.size(); i++) {
        CLog::logDebug("%08x - %08x", charsToEscape[i], (unsigned char)charsToEscape[i]);
    }
}
