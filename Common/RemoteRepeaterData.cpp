/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include "RemoteRepeaterData.h"

CRemoteRepeaterData::CRemoteRepeaterData(const std::string& callsign, RECONNECT reconnect, const std::string& reflector) :
m_callsign(callsign),
m_reconnect(reconnect),
m_reflector(reflector),
m_links()
{
}

CRemoteRepeaterData::~CRemoteRepeaterData()
{
	while (m_links.size()) {
		delete m_links.back();
		m_links.pop_back();
	}
}

void CRemoteRepeaterData::addLink(const std::string& callsign, PROTOCOL protocol, bool linked, DIRECTION direction, bool dongle)
{
	CRemoteLinkData *data = new CRemoteLinkData(callsign, protocol, linked, direction, dongle);
	m_links.push_back(data);
}

std::string CRemoteRepeaterData::getCallsign() const
{
	return m_callsign;
}

int32_t CRemoteRepeaterData::getReconnect() const
{
	return int32_t(m_reconnect);
}

std::string CRemoteRepeaterData::getReflector() const
{
	return m_reflector;
}

unsigned int CRemoteRepeaterData::getLinkCount() const
{
	return m_links.size();
}

CRemoteLinkData *CRemoteRepeaterData::getLink(unsigned int n) const
{
	return m_links[n];
}
