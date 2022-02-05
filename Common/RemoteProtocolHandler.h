/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#ifndef	RemoteProtocolHandler_H
#define	RemoteProtocolHandler_H

#include <string>
#include <vector>

#ifdef USE_STARNET
#include "RemoteStarNetGroup.h"
#endif
#include "RemoteRepeaterData.h"
#include "UDPReaderWriter.h"
#include "Defs.h"


enum RPH_TYPE {
	RPHT_NONE,
	RPHT_LOGIN,
	RPHT_HASH,
	RPHT_CALLSIGNS,
	RPHT_REPEATER,
	RPHT_STARNET,
	RPHT_LINK,
	RPHT_UNLINK,
	RPHT_LINKSCR,
	RPHT_LOGOFF,
	RPHT_LOGOUT,
	RPHT_UNKNOWN
};

class CRemoteProtocolHandler {
public:
	CRemoteProtocolHandler(unsigned int port, const std::string& address = "");
	~CRemoteProtocolHandler();

	bool open();

	RPH_TYPE readType();

	bool     readHash(const std::string& password, uint32_t random);
	std::string readRepeater();
	std::string readStarNetGroup();
	bool     readLink(std::string& callsign, RECONNECT& reconnect, std::string& reflector);
	bool     readUnlink(std::string& callsign, PROTOCOL& protocol, std::string& reflector);
	bool     readLinkScr(std::string& callsign, RECONNECT& reconnect, std::string& reflector);
	bool     readLogoff(std::string& callsign, std::string& user);

	bool     sendACK();
	bool     sendNAK(const std::string& text);
	bool     sendRandom(uint32_t random);
	bool     sendCallsigns(const std::vector<std::string> & repeaters, const std::vector<std::string>& starNets);
	bool     sendRepeater(const CRemoteRepeaterData& data);
#ifdef USE_STARNET
	bool     sendStarNetGroup(const CRemoteStarNetGroup& data);
#endif

	void setLoggedIn(bool set);

	void close();

private:
	CUDPReaderWriter  m_socket;
	in_addr           m_address;
	unsigned int      m_port;
	bool              m_loggedIn;
	RPH_TYPE          m_type;
	unsigned char*    m_inBuffer;
	unsigned int      m_inLength;
	unsigned char*    m_outBuffer;
};

#endif
