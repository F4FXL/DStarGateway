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
#include <boost/circular_buffer.hpp>
#include <chrono>

#include "APRSFrame.h"
#include "RepeaterCallback.h"
#include "Timer.h"
#include "SlowDataEncoder.h"

enum APRSUNIT_STATUS {
    APS_IDLE,
    APS_WAIT,
    APS_TRANSMIT
};

class CAPRSUnit
{
public:
    CAPRSUnit(IRepeaterCallback * repeaterHandler);
    void writeFrame(CAPRSFrame& aprsFrame);
    void clock(unsigned ms);

private:
    // CRingBuffer<CAPRSFrame *> m_frameBuffer;
    boost::circular_buffer<CAPRSFrame *> m_frameBuffer;
    APRSUNIT_STATUS m_status;
    IRepeaterCallback * m_repeaterHandler;
    CHeaderData * m_headerData;
    CSlowDataEncoder *  m_slowData;
    unsigned int m_out;
    unsigned int m_seq;
    unsigned int m_totalNeeded;
    CTimer m_timer;
    std::chrono::high_resolution_clock::time_point m_start;
};


