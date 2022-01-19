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

#include "../../DCSProtocolHandlerPool.h"

namespace DCSProtocolHandlerPoolTests
{
    class DCSProtocolHandler_getHandler : public ::testing::Test {
    
    };

    TEST_F(DCSProtocolHandler_getHandler, successiveCallsReturnsDifferentHandlerAndNotIncoming)
    {
        CDCSProtocolHandlerPool pool(DCS_PORT, "127.0.0.1");

        auto handler1 = pool.getHandler();
        auto handler2 = pool.getHandler();
        auto incoming = pool.getIncomingHandler();

        EXPECT_NE(handler1, nullptr);
        EXPECT_NE(handler2, nullptr);
        EXPECT_NE(incoming, nullptr);

        EXPECT_NE(handler1, handler2);
        EXPECT_NE(handler1, incoming);
        EXPECT_NE(handler2, incoming);

        // DCS_PORT is reserved for incoming handler
        EXPECT_NE(handler1->getPort(), DCS_PORT);
        EXPECT_NE(handler2->getPort(), DCS_PORT);

        EXPECT_EQ(incoming->getPort(), DCS_PORT);

        pool.release(handler1);
        pool.release(handler2);
        pool.release(incoming);
    }
}