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

#include "AMBEFileReader.h"
#include "Log.h"

namespace AMBEFileReaderTests
{
    class AMBEFileReader_read : public ::testing::Test {
    
    };

    TEST_F(AMBEFileReader_read, nonExistentFiles)
    {
        CAMBEFileReader reader("/this/file/does/not/exist", "/neither/does/this/file");
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent files";
    }

    TEST_F(AMBEFileReader_read, onlyIndexFileExists)
    {
        std::string indexFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.indx";
        CAMBEFileReader reader(indexFile, "/this/file/does/not/exist");
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent file";
    }

    TEST_F(AMBEFileReader_read, onlyAmbeFileExists)
    {
        std::string ambeFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.ambe";
        CAMBEFileReader reader("/this/file/does/not/exist", ambeFile);
        bool res = reader.read();
        EXPECT_FALSE(res) << "read shall return false on non existent file";
    }

    TEST_F(AMBEFileReader_read, bothFileExist)
    {
        std::string indexFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.indx";
        std::string ambeFile = std::string(std::filesystem::current_path()) + "/Tests/AMBEFileReader/fr_FR.ambe";

        CLog::logInfo("Reading file: %s", indexFile.c_str());
        CLog::logInfo("Reading file: %s", ambeFile.c_str());

        CAMBEFileReader reader(indexFile, ambeFile);
        bool res = reader.read();
        EXPECT_TRUE(res) << "read shall return true on existent files";
    }
}