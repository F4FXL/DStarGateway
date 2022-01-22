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

#include <netdb.h>

#include "../../NetUtils.h"

namespace NetUtilsTests
{
    class NetUtils_lookup: public ::testing::Test {
    
    };

    TEST_F(NetUtils_lookup, googleShallAlwaysSucceed)
    {
        sockaddr_storage addr;
        struct addrinfo hints;
        ::memset(&hints, 0, sizeof(hints));

        bool res = CNetUtils::lookup("google.fr", addr, hints);

        bool familyOk = addr.ss_family == AF_INET6 || addr.ss_family == AF_INET;
        EXPECT_TRUE(familyOk);
        EXPECT_TRUE(res);
    }

    TEST_F(NetUtils_lookup, erroneousAddress)
    {
        sockaddr_storage addr;
        struct addrinfo hints;
        ::memset(&hints, 0, sizeof(hints));

        bool res = CNetUtils::lookup("gfilufgclqsegfuligyhfcguyhfguilfguils4df64sdw46fcq6sfgvd6f6d7f67d6f7c6sd7f6s7gfv6fc7d6f76tf.fr", addr, hints);

        EXPECT_EQ(addr.ss_family, AF_INET);

        auto ptr = (sockaddr_in*)(&addr);

        EXPECT_EQ((uint32_t)(ptr->sin_addr.s_addr), (uint32_t)INADDR_NONE);
        EXPECT_FALSE(res);
    }
}