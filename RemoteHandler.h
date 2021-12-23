/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#pragma once

#include <string>
#include <cstdint>

#include "RemoteProtocolHandler.h"
#include "Timer.h"

class CRemoteHandler {
public:
	CRemoteHandler(const std::string &password, unsigned int port, const std::string &address = std::string(""));
	~CRemoteHandler();

	bool open();

	void process();

	void close();

private:
	std::string            m_password;
	CRemoteProtocolHandler m_handler;
	uint32_t               m_random;

	void sendGroup(const std::string &callsign);
	void link(const std::string &callsign, const std::string &reflector);
	void unlink(const std::string &callsign);
	void logoff(const std::string &callsign, const std::string &user);
};
