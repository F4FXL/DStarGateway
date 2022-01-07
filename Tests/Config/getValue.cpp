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
#include <unistd.h>
#include <string>

#include "../../Config.h"

namespace ConfigTests
{

    class Config_getValue : public ::testing::Test
    {
    protected:
        std::string m_configPath;

        void SetUp() override
        {
            char buf[2048];
            auto size = ::readlink("/proc/self/exe", buf, 2048);
            if(size > 0) {
                m_configPath.assign(buf, size);
                auto lastSlashPos = m_configPath.find_last_of('/');
                m_configPath.resize(lastSlashPos);
                m_configPath.append("/Config/test.cfg");
            }
        }
    };

    TEST_F(Config_getValue, getURL)
    {
        CConfig config(m_configPath);

        bool ret = config.load();
        std::string value;
        config.getValue("[XLX]", "hostfileUrl", value, 0U, 2048U, "http://www.f4fxl.org?src=github");
 
        EXPECT_TRUE(ret);
        EXPECT_STREQ(value.c_str(), "http://xlxapi.rlx.lu/api.php?do=GetXLXDMRMaster");
    }
}