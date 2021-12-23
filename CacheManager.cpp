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

#include "CacheManager.h"
#include "DStarDefines.h"

CCacheManager::CCacheManager() :
m_userCache(),
m_gatewayCache(),
m_repeaterCache()
{
}

CCacheManager::~CCacheManager()
{
}

// returns a new CUserData if there is a user and a and gateway
CUserData *CCacheManager::findUser(const std::string& user)
{
	mux.lock();
	CUserRecord *ur = m_userCache.find(user);
	if (ur == NULL) {
		mux.unlock();
		return NULL;
	}

	std::string gateway(ur->getRepeater()); // it's not a gateway yet
	CRepeaterRecord *rr = m_repeaterCache.find(gateway);
	if (rr == NULL) {
		gateway = ur->getRepeater();
		gateway.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
		gateway.push_back('G');	          // now it's a gateway
	} else
		gateway = rr->getGateway();

	CGatewayRecord *gr = m_gatewayCache.find(gateway);
	if (gr == NULL) {
		mux.unlock();
		return NULL;
	}

	CUserData *userdata =  new CUserData(user, ur->getRepeater(), gr->getGateway(), gr->getAddress());
	mux.unlock();
	return userdata;
}

CGatewayData *CCacheManager::findGateway(const std::string& gateway)
{
	mux.lock();
	CGatewayRecord *gr = m_gatewayCache.find(gateway);
	if (gr == NULL)
		return NULL;

	CGatewayData *gatewaydata = new CGatewayData(gateway, gr->getAddress(), gr->getProtocol());
	mux.unlock();
	return gatewaydata;
}

CRepeaterData* CCacheManager::findRepeater(const std::string& repeater)
{
	mux.lock();
    CRepeaterRecord *rr = m_repeaterCache.find(repeater);
	std::string gateway;
	if (rr == NULL) {
		gateway = repeater;
		gateway.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
		gateway.push_back('G');
	} else {
		gateway = rr->getGateway();
	}

	CGatewayRecord *gr = m_gatewayCache.find(gateway);
	if (gr == NULL) {
		mux.unlock();
		return NULL;
	}

	CRepeaterData *repeaterdata = new CRepeaterData(repeater, gr->getGateway(), gr->getAddress(), gr->getProtocol());
	mux.unlock();
	return repeaterdata;
}

void CCacheManager::updateUser(const std::string& user, const std::string& repeater, const std::string& gateway, const std::string& address, const std::string& timestamp, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock)
{
	mux.lock();
	std::string repeater7 = repeater.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	std::string gateway7  = gateway.substr(0, LONG_CALLSIGN_LENGTH - 1U);

	m_userCache.update(user, repeater, timestamp);

	// Only store non-standard repeater-gateway pairs
	if (repeater7.compare(gateway7))
		m_repeaterCache.update(repeater, gateway);

	m_gatewayCache.update(gateway, address, protocol, addrLock, protoLock);
	mux.unlock();
}

void CCacheManager::updateRepeater(const std::string& repeater, const std::string& gateway, const std::string& address, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock)
{
	mux.lock();
	std::string repeater7 = repeater.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	std::string gateway7  = gateway.substr(0, LONG_CALLSIGN_LENGTH - 1U);

	// Only store non-standard repeater-gateway pairs
	if (repeater7.compare(gateway7))
		m_repeaterCache.update(repeater, gateway);

	m_gatewayCache.update(gateway, address, protocol, addrLock, protoLock);
	mux.unlock();
}

void CCacheManager::updateGateway(const std::string& gateway, const std::string& address, DSTAR_PROTOCOL protocol, bool addrLock, bool protoLock)
{
	mux.lock();
	m_gatewayCache.update(gateway, address, protocol, addrLock, protoLock);
	mux.unlock();
}
