/*
 *   Copyright (C) 2021-2024 by Geoffrey Merck F4FXL / KC3FRA
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
#include <gmock/gmock.h>

#include "Log.h"
#include "LogSeverity.h"
#include "FakeLogTarget.h"

using ::testing::EndsWith;

namespace LogRepeatTests
{
    class LogRepeat: public ::testing::Test {
        protected:
            CFakeLogTarget * m_logTarget;

        void SetUp() override
        {
            m_logTarget = new CFakeLogTarget(LOG_ERROR);
            CLog::addTarget((CLogTarget *)m_logTarget);
        }

        void TearDown() override
        {
            CLog::finalise();
        }
    };

    TEST_F(LogRepeat, TwoMessage) {
        CLog::logError("One Message");
        CLog::logError("Two Message");

        EXPECT_EQ(2, m_logTarget->m_messages.size()) << "There should be exactly two messages in the log.";
        EXPECT_THAT(m_logTarget->m_messages[0].c_str(), EndsWith("[ERROR  ] One Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[1].c_str(), EndsWith("[ERROR  ] Two Message\n"));
    }

    TEST_F(LogRepeat, ThreeIdenticalMessageThreshold1) {
        CLog::getRepeatThreshold() = 1U;
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("One Message");

        EXPECT_EQ(1, m_logTarget->m_messages.size()) << "There should be one message in the log.";
        EXPECT_THAT(m_logTarget->m_messages[0].c_str(), EndsWith("[ERROR  ] One Message\n"));
    }

    TEST_F(LogRepeat, NineIdenticalMessageTwoDifferentThreshold1) {
        CLog::getRepeatThreshold() = 1U;
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("One Message");        
        CLog::logError("One Message");
        CLog::logError("One Message");       
        CLog::logError("One Message");
        CLog::logError("One Message");        
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("Another Message");
        CLog::logError("And here is another Message");

        EXPECT_EQ(4, m_logTarget->m_messages.size()) << "There should be two message in the log.";
        EXPECT_THAT(m_logTarget->m_messages[0].c_str(), EndsWith("[ERROR  ] One Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[1].c_str(), EndsWith("[ERROR  ] Previous message repeated 8 times\n"));
        EXPECT_THAT(m_logTarget->m_messages[2].c_str(), EndsWith("[ERROR  ] Another Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[3].c_str(), EndsWith("[ERROR  ] And here is another Message\n"));
    }


    TEST_F(LogRepeat, ThreeIdenticalMessageThreshold2) {
        CLog::getRepeatThreshold() = 2U;
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("One Message");

        EXPECT_EQ(2, m_logTarget->m_messages.size()) << "There should be two messages in the log.";
        EXPECT_THAT(m_logTarget->m_messages[0].c_str(), EndsWith("[ERROR  ] One Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[1].c_str(), EndsWith("[ERROR  ] One Message\n"));
    }

    TEST_F(LogRepeat, NineIdenticalMessageTwoDifferentThreshold2) {
        CLog::getRepeatThreshold() = 2U;
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("One Message");        
        CLog::logError("One Message");
        CLog::logError("One Message");       
        CLog::logError("One Message");
        CLog::logError("One Message");        
        CLog::logError("One Message");
        CLog::logError("One Message");
        CLog::logError("Another Message");
        CLog::logError("And here is another Message");

        EXPECT_EQ(5, m_logTarget->m_messages.size()) << "There should be two message in the log.";
        EXPECT_THAT(m_logTarget->m_messages[0].c_str(), EndsWith("[ERROR  ] One Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[1].c_str(), EndsWith("[ERROR  ] One Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[2].c_str(), EndsWith("[ERROR  ] Previous message repeated 7 times\n"));
        EXPECT_THAT(m_logTarget->m_messages[3].c_str(), EndsWith("[ERROR  ] Another Message\n"));
        EXPECT_THAT(m_logTarget->m_messages[4].c_str(), EndsWith("[ERROR  ] And here is another Message\n"));
    }
}