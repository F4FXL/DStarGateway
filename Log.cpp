
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

#include <ctime>
#include <sstream>
#include <cassert>

#include "Log.h"
#include "LogConsoleTarget.h"

LOG_SEVERITY CLog::m_level(LS_DEBUG);
bool CLog::m_addedTargets(false);
std::recursive_mutex CLog::m_targetsMutex;
std::vector<CLogTarget *> CLog::m_targets = { new CLogConsoleTarget(LS_DEBUG) };

void CLog::addTarget(CLogTarget* target)
{
    assert(target != nullptr);

    std::lock_guard lockTargets(m_targetsMutex);

    if(!m_addedTargets) {
        // It is the first time we add an external target, clear the default one(s)
        m_addedTargets = true;
        finalise();
    }

    m_targets.push_back(target);
}

void CLog::finalise()
{
    std::lock_guard lockTargets(m_targetsMutex);
    for(auto target : m_targets) {
        delete target;
    }

    m_targets.clear();
}

void CLog::getTimeStamp(std::string & s)
{
    std::time_t now= std::time(0);
    std::tm* now_tm= std::gmtime(&now);
    char buf[64];
    std::strftime(buf, 42, "%Y-%m-%d %T", now_tm);
    s = std::string(buf);
}
