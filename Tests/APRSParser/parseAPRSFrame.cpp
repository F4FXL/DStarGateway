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

#include "APRSParser.h"

namespace APRSParserTests
{
    class APRSParser_parseAPRSFrame : public ::testing::Test {
    
    };

    TEST_F(APRSParser_parseAPRSFrame, EmpyString)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("", aprsFrame);

        EXPECT_FALSE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
        EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
        EXPECT_EQ(aprsFrame.getPath().size(), 0U);
    }

    TEST_F(APRSParser_parseAPRSFrame, NoSourceCallsign)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame(">APRS::F4ABC    Test Message", aprsFrame);

        EXPECT_FALSE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
        EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
        EXPECT_EQ(aprsFrame.getPath().size(), 0U);
    }

    TEST_F(APRSParser_parseAPRSFrame, NoDestCallsign)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("N0CALL>::F4ABC    Test Message", aprsFrame);

        EXPECT_FALSE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
        EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
        EXPECT_EQ(aprsFrame.getPath().size(), 0U);
    }

    TEST_F(APRSParser_parseAPRSFrame, CorrectMessageFrameWithDigipeater)
    {
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

    TEST_F(APRSParser_parseAPRSFrame, CorrectMessageFrameWithoutDigipeater)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("N0CALL>APRS::F4ABC    :Test Message", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F4ABC    :Test Message");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "N0CALL");
        EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }

    TEST_F(APRSParser_parseAPRSFrame, InvalidMessageFrame)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("N0CALL>APRS::F4ABC&@#$:Test Message", aprsFrame);

        EXPECT_FALSE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "");
        EXPECT_EQ(aprsFrame.getType(), APFT_UNKNOWN);
        EXPECT_EQ(aprsFrame.getPath().size(), 0U);
    }

    TEST_F(APRSParser_parseAPRSFrame, ID51)
    {
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

    TEST_F(APRSParser_parseAPRSFrame, telemetryLabels)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F5ZEE-C>APRS::F5ZEE-C  :PARM.PA Temp", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F5ZEE-C  :PARM.PA Temp");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F5ZEE-C");
        EXPECT_EQ(aprsFrame.getType(), APFT_TELEMETRY);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }

    TEST_F(APRSParser_parseAPRSFrame, telemetryEQNS)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F5ZEE-C>APRS::F5ZEE-C  :EQNS.0,0.16016,-40,0,0,0,0,0,0,0,0,0,0,0,0", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F5ZEE-C  :EQNS.0,0.16016,-40,0,0,0,0,0,0,0,0,0,0,0,0");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F5ZEE-C");
        EXPECT_EQ(aprsFrame.getType(), APFT_TELEMETRY);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }

    TEST_F(APRSParser_parseAPRSFrame, telemetryEQNSWithSeqnum)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F5ZEE-C>APRS::F5ZEE-C  :EQNS.0,0.16016,-40,0,0,0,0,0,0,0,0,0,0,0,0{ABCD", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F5ZEE-C  :EQNS.0,0.16016,-40,0,0,0,0,0,0,0,0,0,0,0,0{ABCD");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F5ZEE-C");
        EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }


    TEST_F(APRSParser_parseAPRSFrame, telemetryReport)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F5ZEE-C>APRS:T#581,342,000,000,000,000,00000000", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), "T#581,342,000,000,000,000,00000000");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F5ZEE-C");
        EXPECT_EQ(aprsFrame.getType(), APFT_TELEMETRY);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }

    TEST_F(APRSParser_parseAPRSFrame, ObjectAlive)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F8DSN-15>API510,DSTAR*:;F1ZBV    *091510h4802.40N/00647.12ErPHG7430/A=003182R Vosges 145,6625@-0,6MHz", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ";F1ZBV    *091510h4802.40N/00647.12ErPHG7430/A=003182R Vosges 145,6625@-0,6MHz");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "API510");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F8DSN-15");
        EXPECT_EQ(aprsFrame.getType(), APFT_OBJECT);
        EXPECT_EQ(aprsFrame.getPath().size(), 1);
        EXPECT_STRCASEEQ(aprsFrame.getPath().at(0).c_str(), "DSTAR*");
    }

    TEST_F(APRSParser_parseAPRSFrame, ObjectKilled)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F8DSN-15>API510,DSTAR*:;F1ZBV    _091510h4802.40N/00647.12ErPHG7430/A=003182R Vosges 145,6625@-0,6MHz", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ";F1ZBV    _091510h4802.40N/00647.12ErPHG7430/A=003182R Vosges 145,6625@-0,6MHz");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "API510");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F8DSN-15");
        EXPECT_EQ(aprsFrame.getType(), APFT_OBJECT);
        EXPECT_EQ(aprsFrame.getPath().size(), 1);
        EXPECT_STRCASEEQ(aprsFrame.getPath().at(0).c_str(), "DSTAR*");
    }

    TEST_F(APRSParser_parseAPRSFrame, ObjectInvalid)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F8DSN-15>API510,DSTAR*:;F1ZBV    ~091510h4802.40N/00647.12ErPHG7430/A=003182R Vosges 145,6625@-0,6MHz", aprsFrame);

        EXPECT_FALSE(retVal);
    }

    TEST_F(APRSParser_parseAPRSFrame, ObjectTooShort)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F8DSN-15>API510,DSTAR*:;F1ZBV", aprsFrame);

        EXPECT_FALSE(retVal);
    }

    TEST_F(APRSParser_parseAPRSFrame, messageToSelf)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F4ABC>APRS::F4ABC    :Test Message{ABCD", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F4ABC    :Test Message{ABCD");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F4ABC");
        EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }

    TEST_F(APRSParser_parseAPRSFrame, messageToSelfNoSeqNum)
    {
        CAPRSFrame aprsFrame;
        bool retVal = CAPRSParser::parseFrame("F4ABC>APRS::F4ABC    :Test Message", aprsFrame);

        EXPECT_TRUE(retVal);
        EXPECT_STRCASEEQ(aprsFrame.getBody().c_str(), ":F4ABC    :Test Message");
        EXPECT_STRCASEEQ(aprsFrame.getDestination().c_str(), "APRS");
        EXPECT_STRCASEEQ(aprsFrame.getSource().c_str(), "F4ABC");
        EXPECT_EQ(aprsFrame.getType(), APFT_MESSAGE);
        EXPECT_EQ(aprsFrame.getPath().size(), 0);
    }
}
