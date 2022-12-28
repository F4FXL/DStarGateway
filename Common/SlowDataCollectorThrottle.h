/*
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include "SlowDataCollector.h"
#include "Timer.h"

class CSlowDataCollectorThrottle : public ISlowDataCollector
{
public:
    CSlowDataCollectorThrottle(ISlowDataCollector* collector, unsigned int timeout);
    ~CSlowDataCollectorThrottle();
    std::string getMyCall1() const;
    void setMyCall1(const std::string& mycall);
    std::string getMyCall2() const;
    void setMyCall2(const std::string& mycall);
    bool writeData(const unsigned char* data);
    void sync();
    unsigned int getData(unsigned char* data, unsigned int length);
    bool getData(std::string& data);
    void reset();
    unsigned char getDataType();
    void clock(unsigned int ms);

private:
    ISlowDataCollector* m_collector;
    CTimer m_timer;
    bool m_isFirst;
    bool m_isComplete;
};