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

#pragma once

#include <string>

enum LOG_SEVERITY {
    LS_TRACE = 1,
    LS_DEBUG,
    LS_INFO,
    LS_WARNING,
    LS_ERROR,
    LS_FATAL
};

class CLogTarget
{
public:
    CLogTarget(LOG_SEVERITY logLevel);
    virtual ~CLogTarget();
    void printLog(const std::string& msg);
    LOG_SEVERITY getLevel() { return m_logLevel; }

protected:
    virtual void printLogInt(const std::string& msg) = 0;

private:
    LOG_SEVERITY m_logLevel;
};