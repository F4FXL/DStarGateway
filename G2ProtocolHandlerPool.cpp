/*
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include "Log.h"
#include "G2ProtocolHandlerPool.h"

const unsigned int G2_BUFFER_LENGTH = 255U;

CG2ProtocolHandlerPool::CG2ProtocolHandlerPool(unsigned short port, const std::string& address) :
m_address(address),
m_basePort(port),
m_socket(address, port)
{
    assert(port > 0U);
    m_index = m_pool.end();
}

CG2ProtocolHandlerPool::~CG2ProtocolHandlerPool()
{

}

bool CG2ProtocolHandlerPool::open()
{
    bool res = m_socket.open();
    return res;
}

void CG2ProtocolHandlerPool::close()
{
    for(auto handler : m_pool) {
        delete handler;
    }
    m_pool.clear();
    m_index = m_pool.end();
    m_socket.close();
}

G2_TYPE CG2ProtocolHandlerPool::read()
{
    bool res = true;
    while(res)
        res = readPackets();

    if(m_index == m_pool.end())
        m_index = m_pool.begin();

    while(m_index != m_pool.end()) {
        if((*m_index)->getType() != GT_NONE) {
            return (*m_index)->getType();
        }
        m_index++;
    }

    return GT_NONE;
}

CAMBEData * CG2ProtocolHandlerPool::readAMBE()
{
    if(m_index == m_pool.end() || (*m_index)->getType() != GT_AMBE)
        return nullptr;

    return (*m_index)->readAMBE();
}

CHeaderData * CG2ProtocolHandlerPool::readHeader()
{
    if(m_index == m_pool.end() || (*m_index)->getType() != GT_HEADER)
        return nullptr;

    return (*m_index)->readHeader();
}

bool CG2ProtocolHandlerPool::readPackets()
{
	unsigned char buffer[G2_BUFFER_LENGTH];
    struct sockaddr_storage addr;
    ::memset(&addr, 0, sizeof(sockaddr_storage));

	// No more data?
	int length = m_socket.read(buffer, G2_BUFFER_LENGTH, addr);
    if(length <= 0) return false;

	if(length == 1 && buffer[0] == 0U) {
		CLog::logDebug("G2 Nat traversal packet received");
	}

    CG2ProtocolHandler * handler = findHandler(addr, IMT_ADDRESS_AND_PORT);
    if(handler == nullptr) {
        CLog::logTrace("new incoming G2 %s:%u", inet_ntoa(TOIPV4(addr)->sin_addr), ntohs(TOIPV4(addr)->sin_port));
        handler = new CG2ProtocolHandler(&m_socket, addr, G2_BUFFER_LENGTH);
        m_pool.push_back(handler);
        m_index = m_pool.end();
    }

    bool res = handler->setBuffer(buffer, length);
    return res;
}

void CG2ProtocolHandlerPool::traverseNat(const std::string& address)
{
	unsigned char buffer = 0x00U;
	
	in_addr addr = CUDPReaderWriter::lookup(address);

	CLog::logInfo("G2 Punching hole to %s", address.c_str());

	m_socket.write(&buffer, 1U, addr, G2_DV_PORT);
}

bool CG2ProtocolHandlerPool::writeHeader(const CHeaderData& header)
{
    auto handler = findHandler(header.getDestination(), IMT_ADDRESS_AND_PORT);
    if(handler == nullptr)
        handler = findHandler(header.getDestination(), IMT_ADDRESS_ONLY);

    if(handler == nullptr) {
        handler = new CG2ProtocolHandler(&m_socket, header.getDestination(), G2_BUFFER_LENGTH);
        m_pool.push_back(handler);
        m_index = m_pool.end();
    }
    return handler->writeHeader(header);
}

bool CG2ProtocolHandlerPool::writeAMBE(const CAMBEData& data)
{
    auto handler = findHandler(data.getDestination(), IMT_ADDRESS_AND_PORT);
    if(handler == nullptr)
        handler = findHandler(data.getDestination(), IMT_ADDRESS_ONLY);

    if(handler == nullptr) {
        handler = new CG2ProtocolHandler(&m_socket, data.getDestination(), G2_BUFFER_LENGTH);
        m_pool.push_back(handler);
        m_index = m_pool.end();
    }

    return handler->writeAMBE(data);
}

CG2ProtocolHandler * CG2ProtocolHandlerPool::findHandler(const struct sockaddr_storage& addr, IPMATCHTYPE matchType) const
{
    for(auto handler : m_pool) {
        if(handler != nullptr && CNetUtils::match(addr, handler->getDestination(), matchType))
            return handler;
    }

    return nullptr;
}

CG2ProtocolHandler * CG2ProtocolHandlerPool::findHandler(in_addr addr, unsigned int port, IPMATCHTYPE matchType) const
{
    struct sockaddr_storage addrStorage;
    addrStorage.ss_family = AF_INET;
    TOIPV4(addrStorage)->sin_addr = addr;
    TOIPV4(addrStorage)->sin_port = port;

    return findHandler(addrStorage, matchType);
}

void CG2ProtocolHandlerPool::clock(unsigned int ms)
{
    for(auto it = m_pool.begin(); it != m_pool.end();) {
        (*it)->clock(ms);
        if((*it)->isInactive()) {
            delete (*it);
            it = m_pool.erase(it);
            m_index = m_pool.end();
        }
        else {
            it++;
        }
    }
}