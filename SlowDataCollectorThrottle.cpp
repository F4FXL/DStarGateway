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

#include <cassert>

#include "SlowDataCollectorThrottle.h"

CSlowDataCollectorThrottle::CSlowDataCollectorThrottle(ISlowDataCollector* collector, unsigned int timeout) :
m_collector(collector),
m_timer(1000U, timeout),
m_isFirst(true)
{
    assert(collector != nullptr);
}

CSlowDataCollectorThrottle::~CSlowDataCollectorThrottle()
{
    delete m_collector;
}

std::string CSlowDataCollectorThrottle::getMyCall() const
{
    return m_collector->getMyCall();
}

void CSlowDataCollectorThrottle::setMyCall(const std::string& mycall)
{
    m_isFirst = true;
    m_collector->setMyCall(mycall);
}
bool CSlowDataCollectorThrottle::writeData(const unsigned char* data)
{
    m_isComplete = false;
    bool complete = m_collector->writeData(data);
    if(complete){
        if(m_isFirst) {
            m_isFirst = false;
            m_isComplete = true;
            m_timer.start();
            return true;
        }
        
        if(m_timer.hasExpired()) {
            m_isComplete = true;
            m_timer.start();
            return true;
        }
    }

    return false;
}
void CSlowDataCollectorThrottle::sync()
{
    m_collector->sync();
}
unsigned int CSlowDataCollectorThrottle::getData(unsigned char* data, unsigned int length)
{
    if(m_isComplete)
        return m_collector->getData(data, length);
    
    return 0U;
}
bool CSlowDataCollectorThrottle::getData(std::string& data)
{
    if(m_isComplete)
        return m_collector->getData(data);

    return false;
}
void CSlowDataCollectorThrottle::reset()
{
    m_timer.start();
    m_collector->reset();
}
unsigned char CSlowDataCollectorThrottle::getDataType()
{
    return m_collector->getDataType();
}

void CSlowDataCollectorThrottle::clock(unsigned int ms)
{
    m_timer.clock(ms);
}