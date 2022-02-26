/*
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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
#include <filesystem>
#include <string>
#include <vector>

#include "AMBEFileReader.h"
#include "AMBEData.h"

namespace AMBEFileReaderTests
{
    class AMBEFileReader_lookup : public ::testing::Test {
    
    };

    TEST_F(AMBEFileReader_lookup, nonExistentFiles)
    {
        CAMBEFileReader reader("/this/file/does/not/exist", "/neither/does/this/file");
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent files";

        std::vector<CAMBEData *> data;
        res = reader.lookup("0", data);
        EXPECT_FALSE(res) << "read shall return false on non existent files";

        for(auto d : data) {
            delete d;
        }
    }

    TEST_F(AMBEFileReader_lookup, onlyIndexFileExists)
    {
        std::string indexFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.indx";
        CAMBEFileReader reader(indexFile, "/this/file/does/not/exist");
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent file";

        std::vector<CAMBEData *> data;
        res = reader.lookup("0", data);
        EXPECT_FALSE(res) << "read shall return false on non existent file";

        for(auto d : data) {
            delete d;
        }
    }

    TEST_F(AMBEFileReader_lookup, onlyAmbeFileExists)
    {
        std::string ambeFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.ambe";
        CAMBEFileReader reader("/this/file/does/not/exist", ambeFile);
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent file";

        std::vector<CAMBEData *> data;
        res = reader.lookup("0", data);
        EXPECT_FALSE(res) << "read shall return false on non existent file";

        for(auto d : data) {
            delete d;
        }
    }

    TEST_F(AMBEFileReader_lookup, validId)
    {
        std::string indexFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.indx";
        std::string ambeFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.ambe";
        CAMBEFileReader reader(indexFile, ambeFile);
        bool res = reader.read();
        EXPECT_TRUE(res) << "read shall return true on existent files";

        std::vector<CAMBEData *> data;
        res = reader.lookup("0", data);
        EXPECT_TRUE(res) << "read shall return true on existent files and valid Id";
        EXPECT_NE(data.size(), 0U) << "Vector shall contain data";

        for(auto d : data) {
            delete d;
        }
    }

    TEST_F(AMBEFileReader_lookup, invalidId)
    {
        std::string indexFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.indx";
        std::string ambeFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.ambe";
        CAMBEFileReader reader(indexFile, ambeFile);
        bool res = reader.read();
        EXPECT_TRUE(res) << "read shall return true on existent files";

        std::vector<CAMBEData *> data;
        res = reader.lookup("This Id does not exist", data);
        EXPECT_FALSE(res) << "read shall return false on existent files and invalid Id";
        EXPECT_EQ(data.size(), 0U) << "Vector shall not contain data";

        for(auto d : data) {
            delete d;
        }
    }
}