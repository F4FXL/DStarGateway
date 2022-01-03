/*
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include "../../APRSParser.h"

class APRSParser_parseAPRSFrame_Tests : public ::testing::Test {
 
};

TEST_F(APRSParser_parseAPRSFrame_Tests, EmpyString) {

    TAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("", aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.m_body.c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.m_dest.c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.m_source.c_str(), "");
    EXPECT_EQ(aprsFrame.m_type, APFT_UNKNOWN);
    EXPECT_EQ(aprsFrame.m_path.size(), 0U);
}