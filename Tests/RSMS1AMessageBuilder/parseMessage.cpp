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

#include "../../RSMS1AMessageBuilder.h"

namespace RSMS1AMessageBuilder
{
    class RSMS1AMessageBuilder_parseMessage : public ::testing::Test {
    
    };

    TEST_F(RSMS1AMessageBuilder_parseMessage, NoError)
    {
        std::string message = "$$Msg,KC3FRA,F4FXL,001118Saluto, comment vas tu?z\r";
        std::string sender, recipient, body;

        auto ret = CRSMS1AMessageBuilder::parseMessage(sender, recipient, body, message);

        EXPECT_EQ(ret, RSMS_OK);
        EXPECT_STREQ(sender.c_str(), "KC3FRA");
        EXPECT_STREQ(recipient.c_str(), "F4FXL");
        EXPECT_STREQ(body.c_str(), "Salut, comment vas tu?");
    }

    TEST_F(RSMS1AMessageBuilder_parseMessage, ErrorInMessage)
    {
        std::string message = "$$Msg,KC3FRA,F4FXL,001118Saluto, comlent vas tu?z\r";
        std::string sender, recipient, body;

        auto ret = CRSMS1AMessageBuilder::parseMessage(sender, recipient, body, message);

        EXPECT_EQ(ret, RSMS_ERRONEOUS_MSG);
        EXPECT_STREQ(sender.c_str(), "KC3FRA");
        EXPECT_STREQ(recipient.c_str(), "F4FXL");
        EXPECT_STREQ(body.c_str(), "Salut, comlent vas tu?");
    }

    TEST_F(RSMS1AMessageBuilder_parseMessage, ErrorInHeader)
    {
        std::string message = "$$Msg,KC3FRB,F4FXL,001118Saluto, comment vas tu?z\r";
        std::string sender, recipient, body;

        auto ret = CRSMS1AMessageBuilder::parseMessage(sender, recipient, body, message);

        EXPECT_EQ(ret, RSMS_FAIL);
        EXPECT_STREQ(sender.c_str(), "");
        EXPECT_STREQ(recipient.c_str(), "");
        EXPECT_STREQ(body.c_str(), "");
    }
};