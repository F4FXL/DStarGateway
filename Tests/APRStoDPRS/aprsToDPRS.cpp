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

#include "../../APRStoDPRS.h"

namespace APRStoDPRSTests
{
    class APRStoDPRS_aprsToDPRS : public ::testing::Test {
    
    };

    TEST_F(APRStoDPRS_aprsToDPRS, validMessage)
    {
        CAPRSFrame frame("KC3FRA", "APRS", {"WIDE1-1", "WIDE2-2"}, ":F4FXL    :Salut, comment vas tu?", APFT_MESSAGE);

        std::string dprs, text;
        CHeaderData header;
        bool ret = CAPRSToDPRS::aprsToDPRS(dprs, text, header, frame);

        EXPECT_TRUE(ret);
        EXPECT_STREQ(dprs.c_str(), "$$Msg,KC3FRA,F4FXL,001118Saluto, comment vas tu?z\r");
        EXPECT_STREQ(text.c_str(), "Salut, comment vas tu?");
        EXPECT_STREQ(header.getMyCall1().c_str(), "KC3FRA  ");
        EXPECT_STREQ(header.getMyCall2().c_str(), "MSG ");
        EXPECT_STREQ(header.getYourCall().c_str(), "F4FXL   ");
    }

    TEST_F(APRStoDPRS_aprsToDPRS, validMessageRecipientWithSSID)
    {
        CAPRSFrame frame("KC3FRA", "APRS", {"WIDE1-1", "WIDE2-2"}, ":F4FXL-7  :Salut, comment vas tu?", APFT_MESSAGE);

        std::string dprs, text;
        CHeaderData header;
        bool ret = CAPRSToDPRS::aprsToDPRS(dprs, text, header, frame);

        EXPECT_TRUE(ret);
        EXPECT_STREQ(dprs.c_str(), "$$Msg,KC3FRA,F4FXL,001118Saluto, comment vas tu?z\r");
        EXPECT_STREQ(text.c_str(), "Salut, comment vas tu?");
        EXPECT_STREQ(header.getMyCall1().c_str(), "KC3FRA  ");
        EXPECT_STREQ(header.getMyCall2().c_str(), "MSG ");
        EXPECT_STREQ(header.getYourCall().c_str(), "F4FXL   ");
    }

    TEST_F(APRStoDPRS_aprsToDPRS, emptyRecipient)
    {
        CAPRSFrame frame("KC3FRA", "APRS", {"WIDE1-1", "WIDE2-2"}, ":         :Salut, comment vas tu?", APFT_MESSAGE);

        std::string dprs, text;
        CHeaderData header;
        bool ret = CAPRSToDPRS::aprsToDPRS(dprs, text, header, frame);

        EXPECT_FALSE(ret);
        EXPECT_STREQ(dprs.c_str(), "");
        EXPECT_STREQ(text.c_str(), "");
        EXPECT_STREQ(header.getMyCall1().c_str(), "        ");
        EXPECT_STREQ(header.getMyCall2().c_str(), "    ");
        EXPECT_STREQ(header.getYourCall().c_str(), "        ");
    }
}
