/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
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

#include "RemoteControlRepeaterData.h"


CRemoteControlRepeaterData::CRemoteControlRepeaterData(const std::string& callsign, int32_t reconnect, const std::string& reflector) :
m_callsign(callsign),
m_reconnect(RECONNECT(reconnect)),
m_reflector(reflector),
m_links()
{
}

CRemoteControlRepeaterData::~CRemoteControlRepeaterData()
{
	for(auto data : m_links) {
		delete data;
	}
	m_links.clear();
}

void CRemoteControlRepeaterData::addLink(const std::string& callsign, int32_t protocol, int32_t linked, int32_t direction, int32_t dongle)
{
	CRemoteControlLinkData  * data = new CRemoteControlLinkData(callsign, protocol, linked, direction, dongle);

	m_links.push_back(data);
}

std::string CRemoteControlRepeaterData::getCallsign() const
{
	return m_callsign;
}

RECONNECT CRemoteControlRepeaterData::getReconnect() const
{
	return m_reconnect;
}

std::string CRemoteControlRepeaterData::getReflector() const
{
	return m_reflector;
}

unsigned int CRemoteControlRepeaterData::getLinkCount() const
{
	return m_links.size();
}

CRemoteControlLinkData* CRemoteControlRepeaterData::getLink(unsigned int n) const
{
	return m_links[n];
}
