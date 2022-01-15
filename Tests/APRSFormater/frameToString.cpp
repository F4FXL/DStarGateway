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

#include "../../APRSFormater.h"

 class APRSFormater_frameToString : public ::testing::Test {
 
};

TEST_F(APRSFormater_frameToString, EmptyFrame) {

    CAPRSFrame aprsFrame("", "", { }, "", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STREQ(output.c_str(), "This should be left untouched if the test succeeds");
}

TEST_F(APRSFormater_frameToString, SourceOnly) {

    CAPRSFrame aprsFrame("N0CALL", "", { }, "", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STREQ(output.c_str(), "This should be left untouched if the test succeeds");
}

TEST_F(APRSFormater_frameToString, DestinationOnly) {

    CAPRSFrame aprsFrame("", "APRS", { }, "", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STREQ(output.c_str(), "This should be left untouched if the test succeeds");
}

TEST_F(APRSFormater_frameToString, PathOnly) {

    CAPRSFrame aprsFrame("", "", { "WIDE1-1, WIDE2-1" }, "", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STREQ(output.c_str(), "This should be left untouched if the test succeeds");
}

TEST_F(APRSFormater_frameToString, BodyOnly) {

    CAPRSFrame aprsFrame("", "", { }, "Lorem Ipsum", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_FALSE(retVal);
    EXPECT_STREQ(output.c_str(), "This should be left untouched if the test succeeds");
}

TEST_F(APRSFormater_frameToString, CorrectWithoutPath) {

    CAPRSFrame aprsFrame("N0CALL", "APRS", { }, "Lorem Ipsum", APFT_UNKNOWN);
    std::string output("This should NOT be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STREQ(output.c_str(), "N0CALL>APRS:Lorem Ipsum");
}

TEST_F(APRSFormater_frameToString, CorrectWithPath) {

    CAPRSFrame aprsFrame("N0CALL", "APRS", { "WIDE1-1", "WIDE2-2" }, "Lorem Ipsum", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STREQ(output.c_str(), "N0CALL>APRS,WIDE1-1,WIDE2-2:Lorem Ipsum");
}

TEST_F(APRSFormater_frameToString, WithSomeEmptyPath) {

    CAPRSFrame aprsFrame("N0CALL", "APRS", { "WIDE1-1", "", "WIDE2-2" }, "Lorem Ipsum", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STREQ(output.c_str(), "N0CALL>APRS,WIDE1-1,WIDE2-2:Lorem Ipsum");
}

TEST_F(APRSFormater_frameToString, WithSomeBlankPath) {

    CAPRSFrame aprsFrame("N0CALL", "APRS", { "WIDE1-1", "", "WIDE2-2" }, "Lorem Ipsum", APFT_UNKNOWN);
    std::string output("This should be left untouched if the test succeeds");
    bool retVal = CAPRSFormater::frameToString(output, aprsFrame);

    EXPECT_TRUE(retVal);
    EXPECT_STREQ(output.c_str(), "N0CALL>APRS,WIDE1-1,WIDE2-2:Lorem Ipsum");
}