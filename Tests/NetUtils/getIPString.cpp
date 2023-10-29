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

#include <netdb.h>

#include "NetUtils.h"

namespace NetUtilsTests
{
    class NetUtils_getIPString: public ::testing::Test {
    
    };

    TEST_F(NetUtils_getIPString, anyAddrV4)
    {
        sockaddr_storage addr;
        std::string s;

        TOIPV4(addr)->sin_addr.s_addr = INADDR_ANY;

        CNetUtils::getIPString(addr, s);

        EXPECT_STREQ(s.c_str(), "0.0.0.0");
    }
}