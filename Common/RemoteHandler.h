/*
 *   Copyright (C) 2010,2012 by Jonathan Naylor G4KLX
 *   copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#ifndef	RemoteHandler_H
#define	RemoteHandler_H

#include <string>

#include "RemoteProtocolHandler.h"
#include "Timer.h"


class CRemoteHandler {
public:
	CRemoteHandler(const std::string& password, unsigned int port, const std::string& address = "");
	~CRemoteHandler();

	bool open();

	void process();

	void close();

private:
	std::string               m_password;
	CRemoteProtocolHandler m_handler;
	unsigned int           m_random;

	void sendCallsigns();
	void sendRepeater(const std::string& callsign);
#if USE_STARNET
	void sendStarNetGroup(const std::string& callsign);
#endif
	void link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector, bool respond);
	void unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector);
#if USE_STARNET
	void logoff(const std::string& callsign, const std::string& user);
#endif
};

#endif
