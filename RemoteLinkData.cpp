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

#include "RemoteLinkData.h"

CRemoteLinkData::CRemoteLinkData(const std::string& callsign, PROTOCOL protocol, bool linked, DIRECTION direction, bool dongle) :
m_callsign(callsign),
m_protocol(protocol),
m_linked(linked),
m_direction(direction),
m_dongle(dongle)
{
}

CRemoteLinkData::~CRemoteLinkData()
{
}

std::string CRemoteLinkData::getCallsign() const
{
	return m_callsign;
}

int32_t CRemoteLinkData::getProtocol() const
{
	return int32_t(m_protocol);
}

int32_t CRemoteLinkData::isLinked() const
{
	return m_linked ? 1 : 0;
}

int32_t CRemoteLinkData::getDirection() const
{
	return int32_t(m_direction);
}

int32_t CRemoteLinkData::isDongle() const
{
	return m_dongle ? 1 : 0;
}
