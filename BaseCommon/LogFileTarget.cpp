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


#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <cassert>

#include "LogFileTarget.h"

CLogFileTarget::CLogFileTarget(LOG_SEVERITY logLevel, const std::string & dir,  const std::string& fileRoot, bool rotate) : 
CLogTarget(logLevel),
m_dir(dir),
m_fileRoot(fileRoot),
m_rotate(rotate),
m_file(),
m_day(0)
{
    assert(!fileRoot.empty());
}

CLogFileTarget::~CLogFileTarget()
{
    if(m_file.is_open()) {
        m_file.close();
    }
}

void CLogFileTarget::printLogIntFixed(const std::string& msg)
{
    if(m_file.is_open()) {
        m_file << msg;
        m_file.flush();
        return;
    }

    std::string filename(m_dir);
    if(filename[filename.length() - 1U] != '/') filename.push_back('/');
    filename.append(m_fileRoot).append(".log");
    m_file.open(filename, std::ios::app);

    if(m_file.is_open()) {
        printLogIntFixed(msg);
    }
    else {
        std::cerr << "FAILED TO OPEN LOG FILE :" << filename;
    }
}

void CLogFileTarget::printLogIntRotate(const std::string& msg)
{
    std::time_t now = std::time(0);
    std::tm* now_tm = std::gmtime(&now);
    if(now_tm->tm_yday != m_day) {
        m_day = now_tm->tm_yday;
        if(m_file.is_open()) {
            m_file.close();
        }
    }

    if(!m_file.is_open()) {
        std::string filename(m_dir);
        if(filename[filename.length() - 1U] != '/') filename.push_back('/');
        char buf[64];
        std::strftime(buf, 42, "-%Y-%m-%d", now_tm);
        filename.append(m_fileRoot).append(buf).append(".log");
        m_file.open(filename, std::ios::app);
        if(!m_file.is_open()) {
            std::cerr << "FAILED TO OPEN LOG FILE :" << filename;
        }
    }

    if(m_file.is_open()) {
        m_file << msg;
        m_file.flush();
        return;
    }
}
void CLogFileTarget::printLogInt(const std::string& msg)
{
    m_file.seekp(0, std::ios::end);
    if(m_rotate)
        printLogIntRotate(msg);
    else
        printLogIntFixed(msg);
}
