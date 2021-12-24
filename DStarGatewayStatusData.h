/*
 *   Copyright (C) 2010,2011,2013 by Jonathan Naylor G4KLX
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

#include "Defs.h"

class CIRCDDBGatewayStatusData {
public:
	CIRCDDBGatewayStatusData(IRCDDB_STATUS ircDDBStatus, bool dprsStatus);
	~CIRCDDBGatewayStatusData();

	void setRepeater(unsigned int n, const std::string& callsign, LINK_STATUS linkStatus, const std::string& linkCallsign, const std::string& incoming);

	void setDongles(const std::string& dongles);

	IRCDDB_STATUS getIrcDDBStatus() const;
	bool          getDPRSStatus() const;

	std::string      getCallsign(unsigned int n) const;
	LINK_STATUS   getLinkStatus(unsigned int n) const;
	std::string      getLinkCallsign(unsigned int n) const;
	std::string      getIncoming(unsigned int n) const;

	std::string      getDongles() const;

private:
	IRCDDB_STATUS  m_ircDDBStatus;
	bool           m_dprsStatus;
	std::string       m_callsign[4];
	LINK_STATUS    m_linkStatus[4];
	std::string       m_linkCallsign[4];
	std::string       m_incoming[4];
	std::string       m_dongles;
};

