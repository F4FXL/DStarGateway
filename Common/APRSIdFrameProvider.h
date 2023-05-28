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

#include <vector>

#include "Timer.h"
#include "APRSEntry.h"
#include "APRSFrame.h"

class CAPRSIdFrameProvider
{
public:
    CAPRSIdFrameProvider(const std::string& gateway, unsigned int timeOut);
    virtual ~CAPRSIdFrameProvider();

    bool buildAPRSFrames(const CAPRSEntry * aprsEntry, std::vector<CAPRSFrame *>& frames);
    void clock(unsigned int ms) { m_timer.clock(ms); }
    bool wantsToSend();
    virtual void start() { };
    virtual void close() { };

protected:
    virtual bool buildAPRSFramesInt(const CAPRSEntry * aprsEntry, std::vector<CAPRSFrame *>& frames) = 0;

    void setTimeout(unsigned int timeout)
    { 
        m_timer.start(timeout);
    }

protected:
    std::string m_gateway;
private:
    CTimer m_timer;
};