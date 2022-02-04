/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#ifndef	RemoteControlRemoteControlHandler_H
#define	RemoteControlRemoteControlHandler_H

#include "RemoteControlRepeaterData.h"
#include "RemoteControlStarNetGroup.h"
#include "RemoteControlCallsignData.h"
#include "UDPReaderWriter.h"

enum RC_TYPE {
	RCT_NONE,
	RCT_ACK,
	RCT_NAK,
	RCT_RANDOM,
	RCT_CALLSIGNS,
	RCT_REPEATER,
	RCT_STARNET
};

class CRemoteControlRemoteControlHandler {
public:
	CRemoteControlRemoteControlHandler(const std::string& address, unsigned int port);
	~CRemoteControlRemoteControlHandler();

	bool open();

	RC_TYPE readType();

	std::string                    readNAK();
	unsigned int                readRandom();
	CRemoteControlCallsignData* readCallsigns();
	CRemoteControlRepeaterData* readRepeater();
	CRemoteControlStarNetGroup* readStarNetGroup();

	bool login();
	bool sendHash(const unsigned char* hash, unsigned int length);

	void setLoggedIn(bool set);

	bool getCallsigns();
	bool getRepeater(const std::string& callsign);
	bool getStarNet(const std::string& callsign);

	bool link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector);
	bool unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector);
	bool logoff(const std::string& callsign, const std::string& user);

	bool logout();

	bool retry();

	void close();

private:
	CUDPReaderWriter  m_socket;
	in_addr           m_address;
	unsigned int      m_port;
	bool              m_loggedIn;
	unsigned int      m_retryCount;
	RC_TYPE           m_type;
	unsigned char*    m_inBuffer;
	unsigned int      m_inLength;
	unsigned char*    m_outBuffer;
	unsigned int      m_outLength;
};

#endif
