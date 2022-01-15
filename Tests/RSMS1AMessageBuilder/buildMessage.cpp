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

namespace RSMS1AMessageBuilderTests
{

    class RSMS1AMessageBuilder_buildMessage : public ::testing::Test {
    
    };

    TEST_F(RSMS1AMessageBuilder_buildMessage, ABC)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", "ABC");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118ABCF\r");
    }

    TEST_F(RSMS1AMessageBuilder_buildMessage, A)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", "A");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118AA\r");
    }

    TEST_F(RSMS1AMessageBuilder_buildMessage, AA)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", "AA");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118AA\02\r");
    }

    TEST_F(RSMS1AMessageBuilder_buildMessage, SalutCommentVasTu)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", "Salut, comment vas tu?");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118Saluto, comment vas tu?z\r");
    }

    TEST_F(RSMS1AMessageBuilder_buildMessage, escapeComma)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", ",");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118o,o,\r");
    }

    TEST_F(RSMS1AMessageBuilder_buildMessage, INeedMoreDollars)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL", "I need more $$$$");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL,001118I need more o$o$o$o$\x08\r");
    }

    //"$$Msg,KC3FRA,F4FXL 7,00116Fhello\024\r"
    TEST_F(RSMS1AMessageBuilder_buildMessage, hello)
    {
        std::string message;
        CRSMS1AMessageBuilder::buildMessage(message, "KC3FRA", "F4FXL 7", "hello");

        EXPECT_STREQ(message.c_str(), "$$Msg,KC3FRA,F4FXL 7,00116Fhello\024\r");
    }
};