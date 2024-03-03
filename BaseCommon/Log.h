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


#pragma once

#include <ctime>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <mutex>
#include <cassert>

#include "StringUtils.h"
#include "LogTarget.h"

class CLog
{
private:
    static std::vector<CLogTarget *> m_targets;
    static bool m_addedTargets;
    static std::recursive_mutex m_targetsMutex;
    static std::string m_prevMsg;
    static uint m_prevMsgCount;
    static uint m_repeatThreshold;

    static void getTimeStamp(std::string& s);

    template<typename... Args>
    static void formatLogMessage(std::string& output, LOG_SEVERITY severity, const std::string & f, Args... args)
    {
        assert(severity != LOG_NONE);
        
        std::string severityStr("       ");
        switch (severity)
        {
        case LOG_DEBUG:
            severityStr.assign("DEBUG  ");
            break;
        case LOG_ERROR:
            severityStr.assign("ERROR  ");
            break;
        case LOG_FATAL:
            severityStr.assign("FATAL  ");
            break;
        case LOG_INFO :
            severityStr.assign("INFO   ");
            break;
        case LOG_WARNING:
            severityStr.assign("WARNING");
            break;
        case LOG_TRACE:
            severityStr.assign("TRACE  ");
            break;
        default:
            break;
        }

        std::string f2("[%s] ");
        f2.append(f);
        CStringUtils::string_format_in_place(output, f2, severityStr.c_str(), args...);
        boost::trim_if(output, [](char c){ return c == '\n' || c == '\r' || c == ' ' || c == '\t'; });
        output.push_back('\n');
    }

public:
    static void addTarget(CLogTarget * target);
    static void finalise();
    static uint& getRepeatThreshold();

    template<typename... Args> static void logTrace(const std::string & f, Args... args)
    {
        log(LOG_TRACE, f, args...);
    }

    template<typename... Args> static void logDebug(const std::string & f, Args... args)
    {
        log(LOG_DEBUG, f, args...);
    }

    template<typename... Args> static void logInfo(const std::string & f, Args... args)
    {
        log(LOG_INFO, f, args...);
    }

    template<typename... Args> static void logWarning(const std::string & f, Args... args)
    {
        log(LOG_WARNING, f, args...);
    }

    template<typename... Args> static void logError(const std::string & f, Args... args)
    {
        log(LOG_ERROR, f, args...);
    }

    template<typename... Args> static void logFatal(const std::string & f, Args... args)
    {
        log(LOG_FATAL, f, args...);
    }

    template<typename... Args> static void log(LOG_SEVERITY severity, const std::string & f, Args... args)
    {
        std::lock_guard lockTarget(m_targetsMutex);

        if(m_targets.empty())
            return;

        std::string msg;
        formatLogMessage(msg, severity, f, args...);

        bool repeatedMsg = (msg.compare(m_prevMsg) == 0);

        if(repeatedMsg && m_repeatThreshold > 0U) {
            m_prevMsgCount++;
            if(m_prevMsgCount >= m_repeatThreshold)
                return;
        }
               
        m_prevMsg.assign(msg);

        if(m_prevMsgCount >= m_repeatThreshold && !repeatedMsg && m_repeatThreshold > 0U) {
            formatLogMessage(msg, severity, "Previous message repeated %d times", m_prevMsgCount - m_repeatThreshold + 1);
            m_prevMsg.clear();
        }
        
        std::string timestamp;
        getTimeStamp(timestamp);
        std::string msgts;
        CStringUtils::string_format_in_place(msgts, "[%s] %s", timestamp.c_str(), msg.c_str());

        for(auto target : m_targets) {
            if(severity >= target->getLevel()) {
                target->printLog(msgts);
            }
        }

        if(m_prevMsgCount != 0 && !repeatedMsg) {
            m_prevMsgCount = 0;
            log(severity, f, args ...);
        }
    }
};
