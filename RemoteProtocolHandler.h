/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017,2018 by Thomas A. Early N7TAE
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
#include <list>

#include "RemoteGroup.h"
#include "RemoteRepeaterData.h"
#include "UDPReaderWriter.h"
#include "Defs.h"

enum RPH_TYPE {
	RPHT_NONE,
	RPHT_LOGIN,
	RPHT_HASH,
	RPHT_CALLSIGNS,
	RPHT_REPEATER,
	RPHT_SMARTGROUP,
	RPHT_LINK,
	RPHT_UNLINK,
	RPHT_LINKSCR,
	RPHT_LOGOFF,
	RPHT_LOGOUT,
	RPHT_UNKNOWN
};

class CRemoteProtocolHandler {
public:
	CRemoteProtocolHandler(unsigned int port, const std::string &address = std::string(""));
	~CRemoteProtocolHandler();

	bool open();

	RPH_TYPE readType();

	std::string readRepeater();
	std::string readGroup();
	bool readHash(const std::string &password, uint32_t random);
	bool readLink(std::string &callsign, std::string &reflector);
	bool readUnlink(std::string &callsign);
	bool readLogoff(std::string &callsign, std::string &user);

	bool sendACK();
	bool sendNAK(const std::string &text);
	bool sendRandom(uint32_t random);
	bool sendCallsigns(const std::list<std::string> &repeaters, const std::list<std::string> &groups);
	bool sendRepeater(const CRemoteRepeaterData &data);
	bool sendGroup(const CRemoteGroup &data);

	void setLoggedIn(bool set);

	void close();

private:
	CUDPReaderWriter  m_socket;
	in_addr           m_address;
	unsigned int      m_port;
	bool              m_loggedIn;
	RPH_TYPE          m_type;
	unsigned char    *m_inBuffer;
	unsigned int      m_inLength;
	unsigned char    *m_outBuffer;
};
