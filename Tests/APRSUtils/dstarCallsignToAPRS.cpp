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

#include "APRSUtils.h"

namespace APRSUtilsTests
{
    class APRSUtils_dstarCallsignToAPRS : public ::testing::Test { 
    };

    TEST_F(APRSUtils_dstarCallsignToAPRS, noBlanks)
    {
        std::string call("N0CALL");
        CAPRSUtils::dstarCallsignToAPRS(call);

        EXPECT_STRCASEEQ(call.c_str(), "N0CALL") << "Call shall not have changed";
    }

    TEST_F(APRSUtils_dstarCallsignToAPRS, twoBlanks)
    {
        std::string call("F4ABC  B");
        CAPRSUtils::dstarCallsignToAPRS(call);

        EXPECT_STRCASEEQ(call.c_str(), "F4ABC-B") << "-H shall have been removed";
    }

    TEST_F(APRSUtils_dstarCallsignToAPRS, threeBlanks)
    {
        std::string call("F4AB   B");
        CAPRSUtils::dstarCallsignToAPRS(call);

        EXPECT_STRCASEEQ(call.c_str(), "F4AB-B") << "-H shall have been removed";
    }
}