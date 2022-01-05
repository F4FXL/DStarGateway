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

class APRSParser_parseAPRSFrame : public ::testing::Test {
 
};

TEST_F(APRSParser_parseAPRSFrame, EmpyString) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("", aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
    EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
    EXPECT_EQ(aprsFrame.getPath().size(), 0U);
}

TEST_F(APRSParser_parseAPRSFrame, NoSourceCallsign) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame(">APRS::F4ABC    Test Message", aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
    EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
    EXPECT_EQ(aprsFrame.getPath().size(), 0U);
}

TEST_F(APRSParser_parseAPRSFrame, NoDestCallsign) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("N0CALL>::F4ABC    Test Message", aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
    EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
    EXPECT_EQ(aprsFrame.getPath().size(), 0U);
}

TEST_F(APRSParser_parseAPRSFrame, CorrectMessageFrameWithDigipeater) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("N0CALL>APRS,WIDE1-1,WIDE2-2::F4ABC    :Test Message", aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F4ABC    :Test Message");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "N0CALL");
    EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
    EXPECT_EQ(aprsFrame.getPath().size(), 2);
    EXPECT_STREQ(aprsFrame.getPath()[0].c_str(), "WIDE1-1");
    EXPECT_STREQ(aprsFrame.getPath()[1].c_str(), "WIDE2-2");
}

TEST_F(APRSParser_parseAPRSFrame, CorrectMessageFrameWithoutDigipeater) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("N0CALL>APRS::F4ABC    :Test Message", aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F4ABC    :Test Message");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "N0CALL");
    EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
    EXPECT_EQ(aprsFrame.getPath().size(), 0);
}

TEST_F(APRSParser_parseAPRSFrame, InvalidMessageFrame) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("N0CALL>APRS::F4ABC&@#$:Test Message", aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
    EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
    EXPECT_EQ(aprsFrame.getPath().size(), 0U);
}

// 

TEST_F(APRSParser_parseAPRSFrame, ID51) {

    CAPRSFrame aprsFrame;
    bool retVal = CAPRSParser::parseFrame("F4FXL-8>API51,DSTAR:!1234.56N/12345.67E[/A=000886QRV DStar\r\r\n", aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "!1234.56N/12345.67E[/A=000886QRV DStar\r\r\n");
    EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "API51");
    EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F4FXL-8");
    EXPECT_EQ(aprsFrame.getType(), APFT_POSITION);
    EXPECT_EQ(aprsFrame.getPath().size(), 1);
    EXPECT_STREQ(aprsFrame.getPath()[0].c_str(), "DSTAR");
}