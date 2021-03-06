/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early
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

#include <cassert>

#include "DPlusProtocolHandlerPool.h"
#include "Utils.h"
#include "Log.h"

CDPlusProtocolHandlerPool::CDPlusProtocolHandlerPool(const unsigned int port, const std::string &addr) :
m_basePort(port),
m_address(addr)
{
	assert(port > 0U);
	m_index = m_pool.end();
	CLog::logInfo("DExtra UDP port base = %u\n", port);
}

CDPlusProtocolHandlerPool::~CDPlusProtocolHandlerPool()
{
	while (m_pool.end() != m_pool.begin()) {
		auto it = m_pool.begin();
		delete it->second;
		m_pool.erase(it);
	}
}

CDPlusProtocolHandler* CDPlusProtocolHandlerPool::getIncomingHandler()
{
	auto it = m_pool.find(m_basePort);
	if(it != m_pool.end())
		return it->second;

	return getHandler(m_basePort);
}

CDPlusProtocolHandler* CDPlusProtocolHandlerPool::getHandler()
{
	return getHandler(m_basePort + 1U);
}

CDPlusProtocolHandler* CDPlusProtocolHandlerPool::getHandler(unsigned int port)
{
	while (m_pool.end() != m_pool.find(port))
		port++;	// find an unused port

	CDPlusProtocolHandler *proto = new CDPlusProtocolHandler(port, m_address);
	if (proto) {
		if (proto->open()) {
			m_pool[port] = proto;
			CLog::logInfo("New D Plus Protocol Handler now on UDP port %u.\n", port);
		} else {
			delete proto;
			proto = NULL;
			CLog::logError("Can't open new DPlus UDP port %u!\n", port);
		}
	} else
		CLog::logError("Can't allocate new DPlus ProtocolHandler at port %u\n", port);
	return proto;
}

void CDPlusProtocolHandlerPool::release(CDPlusProtocolHandler *handler)
{
	assert(handler != NULL);
	for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
		if (it->second == handler) {
			m_pool.erase(it);
			handler->close();
			delete handler;
			m_index = m_pool.end(); // m_index might be ut of order so reset it
			CLog::logInfo("Releasing DPlus ProtocolHandler on port %u.\n", it->first);
			return;
		}
	}
	// we should never get here!
	CLog::logInfo("ERROR: could not find  DPlus ProtocolHander (port=%u) to release!\n", handler->getPort());
}

DPLUS_TYPE CDPlusProtocolHandlerPool::read()
{
	if (m_index == m_pool.end())
		m_index = m_pool.begin();
	while (m_index != m_pool.end()) {
		DPLUS_TYPE type = m_index->second->read();
		if (type != DP_NONE)
			return type;
		m_index++;
	}
	return DP_NONE;
}

CHeaderData *CDPlusProtocolHandlerPool::readHeader()
{
	return m_index->second->readHeader();
}

CAMBEData *CDPlusProtocolHandlerPool::readAMBE()
{
	return m_index->second->readAMBE();
}

CPollData *CDPlusProtocolHandlerPool::readPoll()
{
	return m_index->second->readPoll();
}

CConnectData *CDPlusProtocolHandlerPool::readConnect()
{
	return m_index->second->readConnect();
}

void CDPlusProtocolHandlerPool::close()
{
	for (auto it=m_pool.begin(); it!=m_pool.end(); it++)
		it->second->close();
}
