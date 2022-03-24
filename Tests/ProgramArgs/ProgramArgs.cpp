/*
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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
#include <unordered_map>
#include <vector>
#include <string>

#include "ProgramArgs.h"

namespace ProgramArgsTests
{
    class ProgramArgs_eatArguments : public ::testing::Test {
    
    };

    TEST_F(ProgramArgs_eatArguments, OnlyExecutable)
    {
        const char *argv[] { "ACME" };
        std::unordered_map<std::string, std::string> namedArgs;
        std::vector<std::string> positionalArgs;

        CProgramArgs::eatArguments(1, argv, namedArgs, positionalArgs);

        EXPECT_EQ(namedArgs.size(), 0U) << "Named Args shall be empty because we did not pass any args";
        EXPECT_EQ(positionalArgs.size(), 0U) << "Positional Args shall be empty because we did not pass any args";
    }

    TEST_F(ProgramArgs_eatArguments, OneNamedArgsWithValue)
    {
        const char *argv[] { "ACME", "-name", "giovanni" };
        std::unordered_map<std::string, std::string> namedArgs;
        std::vector<std::string> positionalArgs;

        CProgramArgs::eatArguments(3, argv, namedArgs, positionalArgs);

        EXPECT_EQ(namedArgs.size(), 1U) << "Named args shall have one argument";
        EXPECT_EQ(namedArgs.count("name"), 1U) << "Named args shall contained name";
        EXPECT_STREQ(namedArgs["name"].c_str(), "giovanni") << "Name shall be giovanni";

        EXPECT_EQ(positionalArgs.size(), 0U) << "Positional Args shall be empty because we did not pass any positional args";
    }

    TEST_F(ProgramArgs_eatArguments, OneNamedArgsWithoutValue)
    {
        const char *argv[] { "ACME", "-name" };
        std::unordered_map<std::string, std::string> namedArgs;
        std::vector<std::string> positionalArgs;

        CProgramArgs::eatArguments(2, argv, namedArgs, positionalArgs);

        EXPECT_EQ(namedArgs.size(), 1U) << "Named args shall have one argument";
        EXPECT_EQ(namedArgs.count("name"), 1U) << "Named args shall contained name";
        EXPECT_STREQ(namedArgs["name"].c_str(), "") << "Name shall be empty";

        EXPECT_EQ(positionalArgs.size(), 0U) << "Positional Args shall be empty because we did not pass any positional args";
    }

    TEST_F(ProgramArgs_eatArguments, OneNamedArgsWithValueSeveralPositionalArgs)
    {
        const char *argv[] { "ACME", "my", "-name", "is", "giovanni", "giorgio" };
        std::unordered_map<std::string, std::string> namedArgs;
        std::vector<std::string> positionalArgs;

        CProgramArgs::eatArguments(6, argv, namedArgs, positionalArgs);

        EXPECT_EQ(namedArgs.size(), 1U) << "Named args shall have one argument";
        EXPECT_EQ(namedArgs.count("name"), 1U) << "Named args shall contained name";
        EXPECT_STREQ(namedArgs["name"].c_str(), "is") << "Name shall be giovanni";

        EXPECT_EQ(positionalArgs.size(), 3U) << "Positional Args shall contain 3 values";
        EXPECT_STREQ(positionalArgs[0].c_str(), "my");
        EXPECT_STREQ(positionalArgs[1].c_str(), "giovanni");
        EXPECT_STREQ(positionalArgs[2].c_str(), "giorgio");
    }
}