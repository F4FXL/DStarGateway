/*
 *   Copyright (C) 2012,2013,2015 by Jonathan Naylor G4KLX
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

#ifndef	DPlusAuthenticator_H
#define	DPlusAuthenticator_H

#include "Thread.h"
#include "TCPReaderWriterClient.h"
#include "CacheManager.h"
#include "Timer.h"


#include <netinet/in.h>

class CDPlusAuthenticator : public CThread {
public:
	CDPlusAuthenticator(const std::string& loginCallsign, const std::string& gatewayCallsign, const std::string& address, CCacheManager* cache);
	virtual ~CDPlusAuthenticator();

	virtual void  start();

	virtual void* Entry();

	virtual void stop();

private:
	std::string       m_loginCallsign;
	std::string       m_gatewayCallsign;
	std::string       m_address;
	CCacheManager* m_cache;
	CTimer         m_timer;
	bool           m_killed;

	bool poll(const std::string& callsign, const std::string& hostname, unsigned int port, unsigned char id);
	bool authenticate(const std::string& callsign, const std::string& hostname, unsigned int port, unsigned char id, bool writeToCache);
	bool read(CTCPReaderWriterClient& socket, unsigned char* buffer, unsigned int len) const;
};

#endif
