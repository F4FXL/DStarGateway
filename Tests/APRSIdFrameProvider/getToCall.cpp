/*
 *   Copyright (c) 2021-2023 by Geoffrey Merck F4FXL / KC3FRA
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

#include "APRSIdFrameProvider.h"

namespace APRSIdFrameProviderTests
{
    class APRSIdFrameProvider_getToCall : public ::testing::Test {
    
    };

    TEST_F(APRSIdFrameProvider_getToCall, RepeaterToCall)
    {
        std::string toCall;
        CAPRSIdFrameProvider::getToCall(GT_REPEATER, toCall);

        EXPECT_STRCASEEQ(toCall.c_str(), "APD5TR") << "Unexpected toCall";
    }

    TEST_F(APRSIdFrameProvider_getToCall, HotspotToCall)
    {
        std::string toCall;
        CAPRSIdFrameProvider::getToCall(GT_HOTSPOT, toCall);

        EXPECT_STRCASEEQ(toCall.c_str(), "APD5TH") << "Unexpected toCall";
    }

    TEST_F(APRSIdFrameProvider_getToCall, DongleToCall)
    {
        std::string toCall;
        CAPRSIdFrameProvider::getToCall(GT_DONGLE, toCall);

        EXPECT_STRCASEEQ(toCall.c_str(), "APD5TD") << "Unexpected toCall";
    }

    TEST_F(APRSIdFrameProvider_getToCall, SmartGroupToCall)
    {
        std::string toCall;
        CAPRSIdFrameProvider::getToCall(GT_SMARTGROUP, toCall);

        EXPECT_STRCASEEQ(toCall.c_str(), "APD5T0") << "Unexpected toCall";
    }

    TEST_F(APRSIdFrameProvider_getToCall, UnknownToCall)
    {
        std::string toCall;
        CAPRSIdFrameProvider::getToCall((GATEWAY_TYPE)123456789, toCall);

        EXPECT_STRCASEEQ(toCall.c_str(), "APD5T0");
    }
}
