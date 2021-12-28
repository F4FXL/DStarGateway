/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early
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

#include "DExtraProtocolHandlerPool.h"
#include "Utils.h"
#include "Log.h"

CDExtraProtocolHandlerPool::CDExtraProtocolHandlerPool(const unsigned int port, const std::string &addr) :
m_basePort(port),
m_address(addr)
{
	assert(port > 0U);
	m_index = m_pool.end();
	CLog::logInfo("DExtra UDP port base = %u\n", port);
}

CDExtraProtocolHandlerPool::~CDExtraProtocolHandlerPool()
{
	while (m_pool.end() != m_pool.begin()) {
		auto it = m_pool.begin();
		delete it->second;
		m_pool.erase(it);
	}
}

CDExtraProtocolHandler* CDExtraProtocolHandlerPool::getIncomingHandler()
{
	return getHandler(m_basePort);
}

CDExtraProtocolHandler* CDExtraProtocolHandlerPool::getHandler()
{
	return getHandler(m_basePort + 1U);
}

CDExtraProtocolHandler* CDExtraProtocolHandlerPool::getHandler(unsigned int port)
{
	while (m_pool.end() != m_pool.find(port))
		port++;	// find an unused port

	CDExtraProtocolHandler *proto = new CDExtraProtocolHandler(port, m_address);
	if (proto) {
		if (proto->open()) {
			m_pool[port] = proto;
			CLog::logInfo("New CDExtraProtocolHandler now on UDP port %u.\n", port);
		} else {
			delete proto;
			proto = NULL;
			CLog::logInfo("ERROR: Can't open new DExtra UDP port %u!\n", port);
		}
	} else
		CLog::logInfo("ERROR: Can't allocate new CDExtraProtocolHandler at port %u\n", port);
	return proto;
}

void CDExtraProtocolHandlerPool::release(CDExtraProtocolHandler *handler)
{
	assert(handler != NULL);
	for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
		if (it->second == handler) {
			m_pool.erase(it);
			handler->close();
			delete handler;
			m_index = m_pool.end(); // m_index might be out of order, reset it
			CLog::logInfo("Releasing DExtra Protocol Handler on port %u.\n", it->first);

			return;
		}
	}
	// we should never get here!
	CLog::logInfo("ERROR: could not find DExtra Protocol Hander (port=%u) to release!\n", handler->getPort());
}

DEXTRA_TYPE CDExtraProtocolHandlerPool::read()
{
	if (m_index == m_pool.end())
		m_index = m_pool.begin();
	while (m_index != m_pool.end()) {
		DEXTRA_TYPE type = m_index->second->read();
		if (type != DE_NONE)
			return type;
		m_index++;
	}
	return DE_NONE;
}

CHeaderData *CDExtraProtocolHandlerPool::readHeader()
{
	return m_index->second->readHeader();
}

CAMBEData *CDExtraProtocolHandlerPool::readAMBE()
{
	return m_index->second->readAMBE();
}

CPollData *CDExtraProtocolHandlerPool::readPoll()
{
	return m_index->second->readPoll();
}

CConnectData *CDExtraProtocolHandlerPool::readConnect()
{
	return m_index->second->readConnect();
}

void CDExtraProtocolHandlerPool::close()
{
	for (auto it=m_pool.begin(); it!=m_pool.end(); it++)
		it->second->close();
}
