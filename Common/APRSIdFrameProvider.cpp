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
#include <boost/algorithm/string.hpp>

#include "APRSIdFrameProvider.h"

CAPRSIdFrameProvider::CAPRSIdFrameProvider(const std::string& gateway, unsigned int timeout) :
m_gateway(),
m_timer(1000U)
{
    assert(!gateway.empty());

    m_timer.start(timeout);

    m_gateway = gateway;
	m_gateway = m_gateway.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	boost::trim(m_gateway);
}

CAPRSIdFrameProvider::~CAPRSIdFrameProvider()
{

}

bool CAPRSIdFrameProvider::buildAPRSFrames(const CAPRSEntry * entry, std::vector<CAPRSFrame *> & frames)
{
    assert(entry != nullptr);

    return buildAPRSFramesInt(entry, frames);
}

bool CAPRSIdFrameProvider::wantsToSend()
{
    if(m_timer.hasExpired())
    {
        m_timer.start();
        return true;
    }

    return false;
}

void CAPRSIdFrameProvider::getToCall(GATEWAY_TYPE gwType, std::string& toCall)
{
    switch(gwType)
    {
        case GT_REPEATER:
            toCall.assign("APD5TR");
            return;
        case GT_HOTSPOT:
            toCall.assign("APD5TH");
            return;
        case GT_DONGLE:
            toCall.assign("APD5TD");
            return;
        default:
            toCall.assign("APD5T0");
    }
}
