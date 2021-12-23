/*
 *   Copyright (C) 2010-2013,2015 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas Early N7TAE
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

#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pwd.h>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cassert>

#include "DStarGatewayThread.h"
#include "GroupHandler.h"
#include "DExtraHandler.h"			// DEXTRA LINK
#include "DCSHandler.h"				// DCS LINK
// #include "DPlusHandler.h"
#include "HeaderData.h"
#include "G2Handler.h"
#include "AMBEData.h"
#include "Utils.h"

const unsigned int REMOTE_DUMMY_PORT = 65015U;

CDStarGatewayThread::CDStarGatewayThread(unsigned int countDExtra, unsigned int countDCS) :
m_countDExtra(countDExtra),
m_countDCS(countDCS),
m_killed(false),
m_stopped(true),
m_callsign(),
m_address(),
m_g2Handler(NULL),
m_irc(NULL),
m_cache(),
m_logEnabled(false),
m_statusTimer(1000U, 1U),		// 1 second
m_lastStatus(IS_DISCONNECTED),
m_remoteEnabled(false),
m_remotePassword(),
m_remotePort(0U),
m_remote(NULL)
{
	CHeaderData::initialise();
	CG2Handler::initialise(MAX_ROUTES);
	CDExtraHandler::initialise(MAX_DEXTRA_LINKS);
	// CDPlusHandler::initialise(MAX_DPLUS_LINKS);
	CDCSHandler::initialise(MAX_DCS_LINKS);
	CRepeaterHandler::initialise(MAX_REPEATERS);
	CStarNetHandler::initialise(MAX_STARNETS, m_name);
	CCCSHandler::initialise(MAX_REPEATERS);
	CAudioUnit::initialise();
	printf("DStarGatewayThread created. DExtra channels: %d, DCS Channels: %d\n", countDExtra, countDCS);
}

CDStarGatewayThread::~CDStarGatewayThread()
{
	CHeaderData::finalise();
	CG2Handler::finalise();
	CGroupHandler::finalise();
	CDExtraHandler::finalise();
	CDCSHandler::finalise();

	printf("DStarGatewayThread destroyed\n");
}

void CDStarGatewayThread::run()
{
	m_g2Handler = new CG2ProtocolHandler(G2_DV_PORT, m_address);
	bool ret = m_g2Handler->open();
	if (!ret) {
		printf("Could not open the G2 protocol handler\n");
		delete m_g2Handler;
		m_g2Handler = NULL;
	}

	// Wait here until we have the essentials to run
	while (!m_killed && (m_g2Handler == NULL || m_irc == NULL || 0==m_callsign.size()))
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

	if (m_killed)
		return;

	m_stopped = false;

	printf("Starting the Smart Group Server thread\n");

	loadReflectors(DEXTRA_HOSTS_FILE_NAME, DP_DEXTRA);
	loadReflectors(DCS_HOSTS_FILE_NAME, DP_DCS);
	CDExtraProtocolHandlerPool dextraPool(DEXTRA_PORT, m_address);
	CDCSProtocolHandlerPool dcsPool(DCS_PORT, m_address);

	CG2Handler::setG2ProtocolHandler(m_g2Handler);

	CDExtraHandler::setCallsign(m_callsign);
	CDExtraHandler::setDExtraProtocolHandlerPool(&dextraPool);
	CDCSHandler::setDCSProtocolHandlerPool(&dcsPool);
	CDCSHandler::setGatewayType(GT_SMARTGROUP);

	CGroupHandler::setCache(&m_cache);
	CGroupHandler::setGateway(m_callsign);
	CGroupHandler::setG2Handler(m_g2Handler);
	CGroupHandler::setIRC(m_irc);
	if (m_countDExtra || m_countDCS)
		CGroupHandler::link();

	if (m_remoteEnabled && m_remotePassword.size() && m_remotePort > 0U) {
		m_remote = new CRemoteHandler(m_remotePassword, m_remotePort);
		bool res = m_remote->open();
		if (!res) {
			delete m_remote;
			m_remote = NULL;
		}
	}

	time_t start;
	time(&start);

	m_statusTimer.start();

	try {
		while (!m_killed) {
			processIrcDDB();
			processG2();
			processDExtra(&dextraPool);
			processDCS(&dcsPool);
			if (m_remote != NULL)
				m_remote->process();

			time_t now;
			time(&now);
			unsigned long ms = (unsigned long)(1000.0 * difftime(now, start));
			time(&start);

			m_statusTimer.clock(ms);

			CG2Handler::clock(ms);
			CGroupHandler::clock(ms);
			CDExtraHandler::clock(ms);
			CDCSHandler::clock(ms);

			std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PER_TIC_MS));
		}
	}
	catch (std::exception& e) {
		printf("Exception raised - \"%s\"\n", e.what());
	}
	catch (...) {
		printf("Unknown exception raised\n");
	}

	printf("Stopping the Smart Group Server thread\n");

	// Unlink from all reflectors
	CDExtraHandler::unlink();
	dextraPool.close();

	// Unlink from all reflectors
	CDCSHandler::unlink();
	dcsPool.close();

	m_g2Handler->close();
	delete m_g2Handler;

	m_irc->close();
	delete m_irc;

	if (m_remote != NULL) {
		m_remote->close();
		delete m_remote;
	}
}

void CDStarGatewayThread::kill()
{
	m_killed = true;
}

void CDStarGatewayThread::setCallsign(const std::string& callsign)
{
	if (!m_stopped)
		return;

	m_callsign = callsign;
}

void CDStarGatewayThread::setAddress(const std::string& address)
{
	m_address = address;
}

void CDStarGatewayThread::addGroup(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent, unsigned int userTimeout, CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch, const std::string& reflector)
{
	CGroupHandler::add(callsign, logoff, repeater, infoText, permanent, userTimeout, callsignSwitch, txMsgSwitch, reflector);
}

void CDStarGatewayThread::setIRC(CIRCDDB* irc)
{
	assert(irc != NULL);

	m_irc = irc;
}

void CDStarGatewayThread::setRemote(bool enabled, const std::string& password, unsigned int port)
{
	if (enabled) {
		m_remoteEnabled  = true;
		m_remotePassword = password;
		m_remotePort     = port;
	} else {
		m_remoteEnabled  = false;
		m_remotePassword = password;
		m_remotePort     = REMOTE_DUMMY_PORT;
	}
}

void CDStarGatewayThread::processIrcDDB()
{
	// Once per second
	if (m_statusTimer.hasExpired()) {
		int status = m_irc->getConnectionState();
		switch (status) {
			case 0:
			case 10:
				if (m_lastStatus != IS_DISCONNECTED) {
					printf("Disconnected from ircDDB\n");
					m_lastStatus = IS_DISCONNECTED;
				}
				break;
			case 7:
				if (m_lastStatus != IS_CONNECTED) {
					printf("Connected to ircDDB\n");
					m_lastStatus = IS_CONNECTED;
				}
				break;
			default:
				if (m_lastStatus != IS_CONNECTING) {
					printf("Connecting to ircDDB\n");
					m_lastStatus = IS_CONNECTING;
				}
				break;
		}

		m_statusTimer.start();
	}

	// Process all incoming ircDDB messages, updating the caches
	for (;;) {
		IRCDDB_RESPONSE_TYPE type = m_irc->getMessageType();

		switch (type) {
			case IDRT_NONE:
				return;

			case IDRT_USER: {
					std::string user, repeater, gateway, address, timestamp;
					bool res = m_irc->receiveUser(user, repeater, gateway, address, timestamp);
					if (!res)
						break;

					if (address.size()) {
						//printf("USER: %s %s %s %s\n", user.c_str(), repeater.c_str(), gateway.c_str(), address.c_str());
						m_cache.updateUser(user, repeater, gateway, address, timestamp, DP_DEXTRA, false, false);
					//} else {
					//	printf("USER: %s has no IP address!\n", user.c_str());
					}
				}
				break;

			case IDRT_REPEATER: {
					std::string repeater, gateway, address;
					bool res = m_irc->receiveRepeater(repeater, gateway, address);
					if (!res)
						break;

					if (address.size()) {
//						printf("REPEATER: %s %s %s\n", repeater.c_str(), gateway.c_str(), address.c_str());
						m_cache.updateRepeater(repeater, gateway, address, DP_DEXTRA, false, false);
//					} else {
//						printf("REPEATER: %s NOT FOUND\n", repeater.c_str());
					}
				}
				break;

			case IDRT_GATEWAY: {
					std::string gateway, address;
					bool res = m_irc->receiveGateway(gateway, address);
					if (!res)
						break;

					CDExtraHandler::gatewayUpdate(gateway, address);

					CDCSHandler::gatewayUpdate(gateway, address);

					if (0 == address.size()) {
//						printf("GATEWAY: %s %s\n", gateway.c_str(), address.c_str());
						m_cache.updateGateway(gateway, address, DP_DEXTRA, false, false);
//					} else {
//						printf("GATEWAY: %s NOT FOUND\n", gateway.c_str());
					}
				}
				break;
		}
	}
}

void CDStarGatewayThread::processDExtra(CDExtraProtocolHandlerPool *dextraPool)
{
	for (;;) {
		DEXTRA_TYPE type = dextraPool->read();

		switch (type) {
			case DE_NONE:
				return;

			case DE_POLL: {
					CPollData* poll = dextraPool->newPoll();
					if (poll != NULL) {
						CDExtraHandler::process(*poll);
						delete poll;
					}
				}
				break;

			case DE_CONNECT: {
					CConnectData* connect = dextraPool->newConnect();
					if (connect != NULL) {
						CDExtraHandler::process(*connect);
						delete connect;
					}
				}
				break;

			case DE_HEADER: {
					CHeaderData* header = dextraPool->newHeader();
					if (header != NULL) {
						// printf("DExtra header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s\n", header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str());
						CDExtraHandler::process(*header);
						delete header;
					}
				}
				break;

			case DE_AMBE: {
					CAMBEData* data = dextraPool->newAMBE();
					if (data != NULL) {
						CDExtraHandler::process(*data);
						delete data;
					}
				}
				break;
		}
	}
}

void CDStarGatewayThread::processDCS(CDCSProtocolHandlerPool *dcsPool)
{
	for (;;) {
		DCS_TYPE type = dcsPool->read();

		switch (type) {
			case DC_NONE:
				return;

			case DC_POLL: {
					CPollData* poll = dcsPool->readPoll();
					if (poll != NULL) {
						CDCSHandler::process(*poll);
						delete poll;
					}
				}
				break;

			case DC_CONNECT: {
					CConnectData* connect = dcsPool->readConnect();
					if (connect != NULL) {
						CDCSHandler::process(*connect);
						delete connect;
					}
				}
				break;

			case DC_DATA: {
					CAMBEData* data = dcsPool->readData();
					if (data != NULL) {
						// printf("DCS header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s\n", header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str());
						CDCSHandler::process(*data);
						delete data;
					}
				}
				break;
		}
	}
}

void CDStarGatewayThread::processG2()
{
	for (;;) {
		G2_TYPE type = m_g2Handler->read();

		switch (type) {
			case GT_NONE:
				return;

			case GT_HEADER: {
					CHeaderData* header = m_g2Handler->readHeader();
					if (header != NULL) {
//printf("G2 header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X\n", header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str(), header->getFlag1(), header->getFlag2(), header->getFlag3());
						CG2Handler::process(*header);
						delete header;
					}
				}
				break;

			case GT_AMBE: {
					CAMBEData* data = m_g2Handler->readAMBE();
					if (data != NULL) {
						CG2Handler::process(*data);
						delete data;
					}
				}
				break;
		}
	}
}

void CDStarGatewayThread::loadReflectors(const std::string fname, DSTAR_PROTOCOL dstarProtocol)
{
	std::string filepath(CFG_DIR);
	filepath += std::string("/") + fname;

	struct stat sbuf;
	if (stat(filepath.c_str(), &sbuf)) {
		printf("%s doesn't exist!\n", filepath.c_str());
		return;
	}

	std::ifstream hostfile;
	hostfile.open(filepath, std::ifstream::in);
	char line[256];
	hostfile.getline(line, 256);
	int count=0, tries=0;
	while (hostfile.good()) {
		const char *space = " \t\r";
		char *first = strtok(line, space);
		if (first) {
			if ('#' != first[0]) {
				tries++;
				char *second = strtok(NULL, space);
				if (second) {
					char *third = strtok(NULL, space);
					if (third && '#'==third[0])
						third = NULL;
					std::string name(first);
					name.resize(7, ' ');
					name.push_back('G');
					struct hostent *he = gethostbyname(second);
					if (he) {
						count++;
						std::string address(inet_ntoa(*(struct in_addr*)(he->h_addr_list[0])));
						m_cache.updateGateway(name, address, dstarProtocol, third?1:0, true);
//						printf("reflector:%s, address:%s lock:%s\n", name.c_str(), address.c_str(), third?"true":"false");
					}
				}
			}
		}
		hostfile.getline(line, 256);
	}

	printf("Loaded %u of %u %s reflectors\n", count, tries, DP_DEXTRA==dstarProtocol?"DExtra":"DCS");
}
