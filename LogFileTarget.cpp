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

#include "LogFileTarget.h"

CLogFileTarget::CLogFileTarget(LOG_SEVERITY logLevel, const std::string & dir, bool rotate) : 
CLogTarget(logLevel),
m_dir(dir),
m_rotate(rotate)
{

}

void CLogFileTarget::printLogInt(const std::string& msg)
{
    // construct filename
    std::string fileName(m_dir);
    if(fileName[fileName.length() - 1U] != '/') fileName.push_back('/');
    fileName.append("dstargateway");

    if(m_rotate) {
        std::time_t now = std::time(0);
        std::tm* now_tm = std::gmtime(&now);
        char buf[64];
        std::strftime(buf, 42, "-%Y-%m-%d", now_tm);
        fileName.append(std::string(buf));
    }
    fileName.append(".log");

    std::ofstream file;
    file.open(fileName, std::ios::app);
    if(file.is_open()) {
        file << msg;
        file.close();
    }
}