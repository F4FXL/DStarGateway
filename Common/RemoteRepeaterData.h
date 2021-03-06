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

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "RemoteLinkData.h"

class CRemoteRepeaterData {
public:
	CRemoteRepeaterData(const std::string& callsign, RECONNECT reconnect, const std::string& reflector);
	~CRemoteRepeaterData();

	void addLink(const std::string& callsign, PROTOCOL protocol, bool linked, DIRECTION direction, bool dongle);

	std::string getCallsign() const;
	int32_t     getReconnect() const;
	std::string getReflector() const;

	unsigned int getLinkCount() const;
	CRemoteLinkData *getLink(unsigned int n) const;

private:
	std::string m_callsign;
	RECONNECT   m_reconnect;
	std::string m_reflector;
	std::vector<CRemoteLinkData *> m_links;
};
