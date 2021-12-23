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

#pragma once

#include <string>
#include <vector>

#include "Defs.h"
#include "RemoteUser.h"

class CRemoteGroup {
public:
	CRemoteGroup(const std::string& callsign, const std::string& logoff, const std::string &repeater, const std::string &infoText, const std::string &linkReflector,
			LINK_STATUS linkStatus, unsigned int userTimeout);
	~CRemoteGroup();

	void addUser(const std::string& callsign, uint32_t timer, uint32_t timeout);

	std::string getCallsign() const;
	std::string getLogoff() const;
	std::string getRepeater() const;
	std::string getInfoText() const;
	std::string getReflector() const;
	LINK_STATUS getLinkStatus() const;
	unsigned int getUserTimeout() const;

	uint32_t getUserCount() const;
	CRemoteUser *getUser(uint32_t n) const;

private:
	std::string m_callsign;
	std::string m_logoff;
	std::string m_repeater;
	std::string m_infoText;
	std::string m_linkReflector;
	LINK_STATUS m_linkStatus;
	unsigned int m_userTimeout;
	std::vector<CRemoteUser *>  m_users;
};
