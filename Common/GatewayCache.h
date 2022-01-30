/*
 *   Copyright (C) 2010,2011,2012 by Jonathan Naylor G4KLX
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
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DStarDefines.h"
#include "Defs.h"

class CGatewayRecord {
public:
	CGatewayRecord(const std::string& gateway, in_addr address, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock) :
	m_gateway(gateway),
	m_address(address),
	m_protocol(DP_UNKNOWN),
	m_addrLock(addrLock),
	m_protoLock(false)
	{
		if (protocol != DP_UNKNOWN) {
			m_protocol  = protocol;
			m_protoLock = protoLock;
		}
	}

	std::string getGateway() const
	{
		return m_gateway;
	}

	in_addr getAddress() const
	{
		return m_address;
	}

	DSTAR_PROTOCOL getProtocol() const
	{
		return m_protocol;
	}

	void setData(in_addr address, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock)
	{
		if (!m_addrLock) {
			m_address  = address;
			m_addrLock = addrLock;
		}

		if (!m_protoLock) {
			if (protocol != DP_UNKNOWN) {
				m_protocol  = protocol;
				m_protoLock = protoLock;
			}
		}
	}

private:
	std::string       m_gateway;
	in_addr        m_address;
	DSTAR_PROTOCOL m_protocol;
	bool           m_addrLock;
	bool           m_protoLock;
};

class CGatewayCache {
public:
	CGatewayCache();
	~CGatewayCache();

	CGatewayRecord* find(const std::string& gateway);

	void update(const std::string& gateway, const std::string& address, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock);

	unsigned int getCount() const;

private:
	std::unordered_map<std::string, CGatewayRecord *> m_cache;
};
