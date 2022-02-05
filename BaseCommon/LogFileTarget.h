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
#include <fstream>

#include "LogTarget.h"

class CLogFileTarget : public CLogTarget
{
public:
    CLogFileTarget(LOG_SEVERITY logLevel, const std::string& directory, bool rotate);
    ~CLogFileTarget();

protected:
    virtual void printLogInt(const std::string& msg);

private:
    void printLogIntRotate(const std::string& msg);
    void printLogIntFixed(const std::string& msg);
    std::string buildFileName();
    std::string m_dir;
    bool m_rotate;
    std::fstream m_file;
    int m_day;
};