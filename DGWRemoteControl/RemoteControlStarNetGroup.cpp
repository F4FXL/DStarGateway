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

#include "RemoteControlStarNetGroup.h"


CRemoteControlStarNetGroup::CRemoteControlStarNetGroup(const std::string& callsign, const std::string& logoff, uint32_t timer, uint32_t timeout) :
m_callsign(callsign),
m_logoff(logoff),
m_timer((unsigned int)timer),
m_timeout((unsigned int)timeout),
m_users()
{
	if (m_logoff == "        ")
		m_logoff.clear();
}

CRemoteControlStarNetGroup::~CRemoteControlStarNetGroup()
{
	for(auto user : m_users) {
		delete user;
	}
	m_users.clear();
}

void CRemoteControlStarNetGroup::addUser(const std::string& callsign, uint32_t timer, uint32_t timeout)
{
	CRemoteControlStarNetUser * user = new CRemoteControlStarNetUser(callsign, timer, timeout);

	m_users.push_back(user);
}

std::string CRemoteControlStarNetGroup::getCallsign() const
{
	return m_callsign;
}

std::string CRemoteControlStarNetGroup::getLogoff() const
{
	return m_logoff;
}

unsigned int CRemoteControlStarNetGroup::getTimer() const
{
	return m_timer;
}

unsigned int CRemoteControlStarNetGroup::getTimeout() const
{
	return m_timeout;
}

unsigned int CRemoteControlStarNetGroup::getUserCount() const
{
	return m_users.size();
}

CRemoteControlStarNetUser * CRemoteControlStarNetGroup::getUser(unsigned int n) const
{
	return m_users[n];
}
