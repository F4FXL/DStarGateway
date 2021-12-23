/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017,2018 by Thomas A. Early N7TAE
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

#include "RemoteGroup.h"

CRemoteGroup::CRemoteGroup(const std::string& callsign, const std::string& logoff, const std::string &repeater, const std::string &infoText,
											const std::string &linkReflector, LINK_STATUS linkStatus, unsigned int userTimeout) :
m_callsign(callsign),
m_logoff(logoff),
m_repeater(repeater),
m_infoText(infoText),
m_linkReflector(linkReflector),
m_linkStatus(linkStatus),
m_userTimeout(userTimeout),
m_users()
{
	if (logoff.compare("        "))
		logoff.empty();
}

CRemoteGroup::~CRemoteGroup()
{
	while (m_users.size()) {
		delete m_users.back();
		m_users.pop_back();
	}
}

void CRemoteGroup::addUser(const std::string& callsign, uint32_t timer, uint32_t timeout)
{
	CRemoteUser *user = new CRemoteUser(callsign, timer, timeout);

	m_users.push_back(user);
}

std::string CRemoteGroup::getCallsign() const
{
	return m_callsign;
}

std::string CRemoteGroup::getLogoff() const
{
	return m_logoff;
}

std::string CRemoteGroup::getRepeater() const
{
	return m_repeater;
}

std::string CRemoteGroup::getInfoText() const
{
	return m_infoText;
}

std::string CRemoteGroup::getReflector() const
{
	return m_linkReflector;
}

LINK_STATUS CRemoteGroup::getLinkStatus() const
{
	return m_linkStatus;
}

unsigned int CRemoteGroup::getUserTimeout() const
{
	return m_userTimeout;
}

uint32_t CRemoteGroup::getUserCount() const
{
	return m_users.size();
}

CRemoteUser *CRemoteGroup::getUser(uint32_t n) const
{
	return m_users[n];
}
