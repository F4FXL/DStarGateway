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

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <mutex>
#include <sstream>

#include "StringUtils.h"
#include "LogTarget.h"

class CLog
{
private:
    static LOG_SEVERITY m_level;
    static std::vector<CLogTarget *> m_targets;
    static bool m_addedTargets;
    static std::recursive_mutex m_targetsMutex;

    static void getTimeStamp(std::string & s);

    template<typename... Args> static void formatLogMessage(std::string& output, LOG_SEVERITY severity, const std::string & f, Args... args)
    {
            std::string severityStr;
            switch (severity)
            {
            case LS_DEBUG:
                severityStr = "DEBUG  ";
                break;
            case LS_ERROR:
                severityStr = "ERROR  ";
                break;
            case LS_FATAL:
                severityStr = "FATAL  ";
                break;
            case LS_INFO :
                severityStr = "INFO   ";
                break;
            case LS_WARNING:
                severityStr = "WARNING";
                break;
            case LS_TRACE:
                severityStr = "TRACE  ";
                break;
            default:
                break;
            }

            std::string message = CStringUtils::string_format(f, args...);
            boost::trim(message);
            std::string timeUtc;
            getTimeStamp(timeUtc);
            std::stringstream s;
            s << "[" <<  timeUtc << "] [" << severityStr << "] " << message << std::endl;

            output = s.str();
    }

public:
    
    static void addTarget(CLogTarget * target);
    static void finalise();

    template<typename... Args> static void logDebug(const std::string & f, Args... args)
    {
        log(LS_DEBUG, f, args...);
    }

    template<typename... Args> static void logInfo(const std::string & f, Args... args)
    {
        log(LS_INFO, f, args...);
    }

    template<typename... Args> static void logWarning(const std::string & f, Args... args)
    {
        log(LS_WARNING, f, args...);
    }

    template<typename... Args> static void logError(const std::string & f, Args... args)
    {
        log(LS_ERROR, f, args...);
    }

    template<typename... Args> static void logFatal(const std::string & f, Args... args)
    {
        log(LS_FATAL, f, args...);
    }

    template<typename... Args> static void log(LOG_SEVERITY severity, const std::string & f, Args... args)
    {
        std::lock_guard lockTarget(m_targetsMutex);

        std::string msg;
        for(auto target : m_targets) {
            if(target->getLevel() >= CLog::m_level) {
                if(msg.empty()) formatLogMessage(msg, severity, f, args...);
                target->printLog(msg);
            }
        }
    }
};
