/*
 *   Copyright (C) 2010,2011,2013 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include "DStarGatewayStatusData.h"

CIRCDDBGatewayStatusData::CIRCDDBGatewayStatusData(IRCDDB_STATUS ircDDBStatus, bool dprsStatus) :
m_ircDDBStatus(ircDDBStatus),
m_dprsStatus(dprsStatus)
{
}

CIRCDDBGatewayStatusData::~CIRCDDBGatewayStatusData()
{
}

void CIRCDDBGatewayStatusData::setRepeater(unsigned int n, const std::string& callsign, LINK_STATUS linkStatus, const std::string& linkCallsign, const std::string& incoming)
{
	assert(n < 4U);

	m_callsign[n]     = callsign;
	m_linkStatus[n]   = linkStatus;
	m_linkCallsign[n] = linkCallsign;
	m_incoming[n]     = incoming;
}

void CIRCDDBGatewayStatusData::setDongles(const std::string& dongles)
{
	m_dongles = dongles;
}

IRCDDB_STATUS CIRCDDBGatewayStatusData::getIrcDDBStatus() const
{
	return m_ircDDBStatus;
}

bool CIRCDDBGatewayStatusData::getDPRSStatus() const
{
	return m_dprsStatus;
}

std::string CIRCDDBGatewayStatusData::getCallsign(unsigned int n) const
{
	assert(n < 4U);

	return m_callsign[n];
}

LINK_STATUS CIRCDDBGatewayStatusData::getLinkStatus(unsigned int n) const
{
	assert(n < 4U);

	return m_linkStatus[n];
}

std::string CIRCDDBGatewayStatusData::getLinkCallsign(unsigned int n) const
{
	assert(n < 4U);

	return m_linkCallsign[n];
}

std::string CIRCDDBGatewayStatusData::getIncoming(unsigned int n) const
{
	assert(n < 4U);

	return m_incoming[n];
}

std::string CIRCDDBGatewayStatusData::getDongles() const
{
	return m_dongles;
}
