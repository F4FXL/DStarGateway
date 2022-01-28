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
    class NetUtils_match: public ::testing::Test {
    
    };

    TEST_F(NetUtils_match, MatchIPAndPort_differentFamilySamePort)
    {
        struct sockaddr_storage addr1, addr2;
        addr1.ss_family = AF_INET6;
        addr2.ss_family = AF_INET;

        ((struct sockaddr_in6 *)&addr1)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr1)->sin6_port = 123;

        ((struct sockaddr_in *)&addr2)->sin_addr.s_addr = INADDR_LOOPBACK;
        ((struct sockaddr_in *)&addr2)->sin_port = 123;

        EXPECT_FALSE(CNetUtils::match(addr1, addr2, IMT_ADDRESS_AND_PORT));
    }

    TEST_F(NetUtils_match, MatchIPAndPort_SameFamilySamePort)
    {
        struct sockaddr_storage addr1, addr2;
        addr1.ss_family = AF_INET6;
        addr2.ss_family = AF_INET6;

        ((struct sockaddr_in6 *)&addr1)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr1)->sin6_port = 123;

        ((struct sockaddr_in6 *)&addr2)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr2)->sin6_port = 123;

        EXPECT_TRUE(CNetUtils::match(addr1, addr2, IMT_ADDRESS_AND_PORT));
    }

    TEST_F(NetUtils_match, MatchIPAndPort_SameFamilyDifferentPort)
    {
        struct sockaddr_storage addr1, addr2;
        addr1.ss_family = AF_INET6;
        addr2.ss_family = AF_INET6;

        ((struct sockaddr_in6 *)&addr1)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr1)->sin6_port = 123;

        ((struct sockaddr_in6 *)&addr2)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr2)->sin6_port = 456;

        EXPECT_FALSE(CNetUtils::match(addr1, addr2, IMT_ADDRESS_AND_PORT));
    }

    TEST_F(NetUtils_match, MatchIP_SameFamilyDifferentPort)
    {
        struct sockaddr_storage addr1, addr2;
        addr1.ss_family = AF_INET6;
        addr2.ss_family = AF_INET6;

        ((struct sockaddr_in6 *)&addr1)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr1)->sin6_port = 123;

        ((struct sockaddr_in6 *)&addr2)->sin6_addr = IN6ADDR_LOOPBACK_INIT;
        ((struct sockaddr_in6 *)&addr2)->sin6_port = 456;

        EXPECT_TRUE(CNetUtils::match(addr1, addr2, IMT_ADDRESS_ONLY));
    }
}