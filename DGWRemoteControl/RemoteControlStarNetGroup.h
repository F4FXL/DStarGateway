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

#ifndef	RemoteControlStarNetGroup_H
#define	RemoteControlStarNetGroup_H

#include <vector>

#include "RemoteControlStarNetUser.h"



class CRemoteControlStarNetGroup {
public:
	CRemoteControlStarNetGroup(const std::string& callsign, const std::string& logoff, uint32_t timer, uint32_t timeout);
	~CRemoteControlStarNetGroup();

	void addUser(const std::string& callsign, uint32_t timer, uint32_t timeout);

	std::string     getCallsign() const;
	std::string     getLogoff() const;
	unsigned int getTimer() const;
	unsigned int getTimeout() const;

	unsigned int getUserCount() const;
	CRemoteControlStarNetUser * getUser(unsigned int n) const;

private:
	std::string           m_callsign;
	std::string           m_logoff;
	unsigned int       m_timer;
	unsigned int       m_timeout;
	std::vector<CRemoteControlStarNetUser *>  m_users;
};

#endif
