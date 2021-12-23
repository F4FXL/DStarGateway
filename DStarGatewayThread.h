/*
 *   Copyright (C) 2010-2015 by Jonathan Naylor G4KLX
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

#include "DExtraProtocolHandlerPool.h"		// DEXTRA_LINK
#include "DCSProtocolHandlerPool.h"			// DCS_LINK
#include "G2ProtocolHandler.h"
#include "RemoteHandler.h"
#include "CacheManager.h"
#include "IRCDDB.h"
#include "Timer.h"
#include "Defs.h"

class CDStarGatewayThread {
public:
	CDStarGatewayThread(unsigned int countDExtra, unsigned int countDCS);
	virtual ~CDStarGatewayThread();

	virtual void setCallsign(const std::string& callsign);
	virtual void setAddress(const std::string& address);

	virtual void addGroup(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent,
							unsigned int userTimeout, CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch, const std::string& reflector);

	virtual void setRemote(bool enabled, const std::string& password, unsigned int port);
	virtual void setIRC(CIRCDDB* irc);

	virtual void run();
	virtual void kill();

private:
	unsigned int m_countDExtra;
	unsigned int m_countDCS;
	bool		m_killed;
	bool		m_stopped;
	std::string	m_callsign;
	std::string	m_address;

	CG2ProtocolHandler *m_g2Handler;
	CIRCDDB            *m_irc;
	CCacheManager 		m_cache;
	bool				m_logEnabled;
	CTimer				m_statusTimer;
	IRCDDB_STATUS		m_lastStatus;
	bool				m_remoteEnabled;
	std::string			m_remotePassword;
	unsigned int		m_remotePort;
	CRemoteHandler     *m_remote;

	void processIrcDDB();
	void processG2();
	void loadReflectors(const std::string fname, DSTAR_PROTOCOL dstarProtocol);

	void processDExtra(CDExtraProtocolHandlerPool *dextraPool);
	void processDCS(CDCSProtocolHandlerPool *dcsPool);
};

