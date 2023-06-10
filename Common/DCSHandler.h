/*
 *   Copyright (C) 2012,2013,2015 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#ifndef	DCSHandler_H
#define	DCSHandler_H

#include <netinet/in.h>
#include <string>
#include <iostream>
#include <fstream>

#include "DCSProtocolHandlerPool.h"
#include "ReflectorCallback.h"
#include "DStarDefines.h"
#include "HeaderLogger.h"
#include "CallsignList.h"
#include "ConnectData.h"
#include "AMBEData.h"
#include "PollData.h"
#include "Timer.h"
#include "Defs.h"

#define GET_DISP_REFLECTOR(refl) (refl->m_isXlx ? refl->m_xlxReflector : refl->m_reflector)

enum DCS_STATE {
	DCS_LINKING,
	DCS_LINKED,
	DCS_UNLINKING
};

class CDCSHandler {
public:
	static void initialise(unsigned int maxReflectors);

	static void setDCSProtocolHandlerPool(CDCSProtocolHandlerPool* pool);
	static void setDCSProtocolIncoming(CDCSProtocolHandler* handler);
	static void setHeaderLogger(CHeaderLogger* logger);
	static void setGatewayType(GATEWAY_TYPE type);

	static void link(IReflectorCallback* handler, const std::string& repeater, const std::string& reflector, const in_addr& address);
	static void unlink(IReflectorCallback* handler, const std::string& reflector = "", bool exclude = true);
	static void unlink();

	static void writeHeader(IReflectorCallback* handler, CHeaderData& header, DIRECTION direction);
	static void writeAMBE(IReflectorCallback* handler, CAMBEData& data, DIRECTION direction);

	static void process(CAMBEData& header);
	static void process(CPollData& data);
	static void process(CConnectData& connect);

	static void gatewayUpdate(const std::string& reflector, const std::string& address);
	static void clock(unsigned int ms);

	static bool stateChange();
	static void writeStatus(std::ofstream& file);

	static void setWhiteList(CCallsignList* list);
	static void setBlackList(CCallsignList* list);

	static void finalise();

	static void getInfo(IReflectorCallback* handler, CRemoteRepeaterData& data);

	static std::string getIncoming(const std::string& callsign);

protected:
	CDCSHandler(IReflectorCallback* handler, const std::string& reflector, const std::string& repeater, CDCSProtocolHandler* protoHandler, const in_addr& address, unsigned int port, DIRECTION direction);
	~CDCSHandler();

	void processInt(CAMBEData& data);
	bool processInt(CConnectData& connect, CD_TYPE type);

	void writeHeaderInt(IReflectorCallback* handler, CHeaderData& header, DIRECTION direction);
	void writeAMBEInt(IReflectorCallback* handler, CAMBEData& data, DIRECTION direction);

	bool clockInt(unsigned int ms);

private:
	static unsigned int             m_maxReflectors;
	static CDCSHandler**            m_reflectors;

	static CDCSProtocolHandlerPool* m_pool;
	static CDCSProtocolHandler*     m_incoming;

	static bool                     m_stateChange;

	static GATEWAY_TYPE             m_gatewayType;

	static CHeaderLogger*           m_headerLogger;

	static CCallsignList*           m_whiteList;
	static CCallsignList*           m_blackList;

	std::string             m_reflector;
	std::string             m_xlxReflector;
	bool                 m_isXlx;	
	std::string             m_repeater;
	CDCSProtocolHandler* m_handler;
	sockaddr_storage	m_yourAddressAndPort;
	unsigned int         m_myPort;
	DIRECTION            m_direction;
	DCS_STATE            m_linkState;
	IReflectorCallback*  m_destination;
	time_t               m_time;
	CTimer               m_pollTimer;
	CTimer               m_pollInactivityTimer;
	CTimer               m_tryTimer;
	unsigned int         m_tryCount;
	unsigned int         m_dcsId;
	unsigned int         m_dcsSeq;
	unsigned int         m_seqNo;
	CTimer               m_inactivityTimer;

	// Header data
	std::string             m_yourCall;
	std::string             m_myCall1;
	std::string             m_myCall2;
	std::string             m_rptCall1;
	std::string             m_rptCall2;

	unsigned int calcBackoff();
};

#endif
