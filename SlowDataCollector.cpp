/*
 *   Copyright (C) 2010,2012,2018 by Jonathan Naylor G4KLX
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
#include <cstring>

#include "SlowDataCollector.h"

const unsigned int SLOW_DATA_BLOCK_LENGTH = 6U;

CSlowDataCollector::CSlowDataCollector(unsigned char slowDataType) :
m_slowDataType(slowDataType),
m_myCall(),
m_state(SS_FIRST)
{
    m_buffer  = new unsigned char[SLOW_DATA_BLOCK_LENGTH];
    std::memset(m_buffer, 0, SLOW_DATA_BLOCK_LENGTH);
}

CSlowDataCollector::~CSlowDataCollector()
{
    delete[] m_buffer;
}

std::string CSlowDataCollector::getMyCall() const
{
    return m_myCall;
}

void CSlowDataCollector::setMyCall(const std::string& myCall)
{
    m_myCall = myCall;
}

bool CSlowDataCollector::writeData(const unsigned char* data)
{
    assert(data != nullptr);

    switch (m_state) {
    case SS_FIRST:
        m_buffer[0U] = data[0U] ^ SCRAMBLER_BYTE1;
        m_buffer[1U] = data[1U] ^ SCRAMBLER_BYTE2;
        m_buffer[2U] = data[2U] ^ SCRAMBLER_BYTE3;
        m_state = SS_SECOND;
        return false;

    case SS_SECOND:
        m_buffer[3U] = data[0U] ^ SCRAMBLER_BYTE1;
        m_buffer[4U] = data[1U] ^ SCRAMBLER_BYTE2;
        m_buffer[5U] = data[2U] ^ SCRAMBLER_BYTE3;
        m_state = SS_FIRST;
        break;
	}

    if((m_buffer[0] & SLOW_DATA_TYPE_MASK) == m_slowDataType)
        return addData(m_buffer + 1U);
    
    return false;
}

void CSlowDataCollector::sync()
{
    m_state = SS_FIRST;
}

unsigned int CSlowDataCollector::getData(unsigned char * data, unsigned int length)
{
    return getDataInt(data, length);
}

void CSlowDataCollector::reset()
{
    m_state = SS_FIRST;
    m_myCall.clear();
    resetInt();
}

unsigned char CSlowDataCollector::getDataType()
{
    return m_slowDataType;
}