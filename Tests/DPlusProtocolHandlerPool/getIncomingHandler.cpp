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

#include "DPlusProtocolHandlerPool.h"

namespace DPlusProtocolHandlerPoolTests
{
    class DPlusProtocolHandler_getIncomingHandler : public ::testing::Test {
    
    };

    TEST_F(DPlusProtocolHandler_getIncomingHandler, successiveCallAlwaysReturnsIncomingHandler)
    {
        CDPlusProtocolHandlerPool pool(DPLUS_PORT, "127.0.0.1");

        auto handler1 = pool.getIncomingHandler();
        auto handler2 = pool.getIncomingHandler();

        EXPECT_NE(handler1, nullptr);
        EXPECT_NE(handler2, nullptr);
        EXPECT_EQ(handler1, handler2);
        EXPECT_EQ(handler1->getPort(), DPLUS_PORT);
        EXPECT_EQ(handler2->getPort(), DPLUS_PORT);

        pool.release(handler1);
        pool.release(handler2);
    }
}