/*
 *   Copyright (C) 2010-2015,2018 by Jonathan Naylor G4KLX
 *	 Copyright (c) 2021 by Geoffrey F4FXL / KC3FRA
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


#include <arpa/inet.h>
#include <chrono>
#include <iostream>
#include <fstream>

#include "DStarGatewayThread.h"
#include "DStarGatewayDefs.h"
#include "RepeaterHandler.h"
#ifdef USE_STARNET
#include "StarNetHandler.h"
#endif
#ifdef USE_CALLSIGN_SERVER
#include "CallsignServer.h"
#endif
#include "DExtraHandler.h"
#include "DPlusHandler.h"
#include "HeaderLogger.h"
#include "ConnectData.h"
#ifdef USE_CCS
#include "CCSHandler.h"
#endif
#include "HeaderData.h"
#include "StatusData.h"
#include "DCSHandler.h"
#include "DDHandler.h"
#include "G2Handler.h"
#include "HeardData.h"
#include "PollData.h"
#include "AMBEData.h"
#include "HostFile.h"
#include "CCSData.h"
#include "DDData.h"
#include "Utils.h"
#include "Defs.h"
#include "Log.h"
#include "StringUtils.h"

const std::string LOOPBACK_ADDRESS("127.0.0.1");

const unsigned int REMOTE_DUMMY_PORT = 65016U;

CDStarGatewayThread::CDStarGatewayThread(const std::string& logDir, const std::string& dataDir, const std::string& name) :
CThread(),
m_logDir(logDir),
m_dataDir(dataDir),
m_name(name),
m_killed(false),
m_stopped(true),
m_gatewayType(GT_REPEATER),
m_gatewayCallsign(),
m_gatewayAddress(),
m_icomRepeaterHandler(NULL),
m_hbRepeaterHandler(NULL),
m_dummyRepeaterHandler(NULL),
m_dextraPool(NULL),
m_dplusPool(NULL),
m_dcsPool(NULL),
m_g2Handler(NULL),
#if defined(ENABLE_NAT_TRAVERSAL)
m_natTraversal(NULL),
#endif
m_aprsWriter(NULL),
m_irc(NULL),
m_cache(),
m_language(TL_ENGLISH_UK),
m_dextraEnabled(true),
m_dextraMaxDongles(0U),
m_dplusEnabled(false),
m_dplusMaxDongles(0U),
m_dplusLogin(),
m_dcsEnabled(true),
m_xlxEnabled(true),
m_xlxHostsFileName(),
m_ccsEnabled(true),
m_ccsHost(),
m_infoEnabled(true),
m_echoEnabled(true),
m_dtmfEnabled(true),
m_logEnabled(false),
m_ddModeEnabled(false),
m_lastStatus(IS_DISABLED),
m_statusTimer1(1000U, 1U),		// 1 second
m_statusTimer2(1000U, 1U),		// 1 second
m_remoteEnabled(false),
m_remotePassword(),
m_remotePort(0U),
m_remote(NULL),
m_statusFileTimer(1000U, 2U * 60U),		// 2 minutes
m_status1(),
m_status2(),
m_status3(),
m_status4(),
m_status5(),
m_latitude(0.0),
m_longitude(0.0),
m_whiteList(NULL),
m_blackList(NULL),
m_restrictList(NULL)
{
	CHeaderData::initialise();
	CG2Handler::initialise(MAX_ROUTES);
	CDExtraHandler::initialise(MAX_DEXTRA_LINKS);
	CDPlusHandler::initialise(MAX_DPLUS_LINKS);
	CDCSHandler::initialise(MAX_DCS_LINKS);
	CRepeaterHandler::initialise(MAX_REPEATERS);
#ifdef USE_STARNET
	CStarNetHandler::initialise(MAX_STARNETS, m_name);
#endif
#ifdef USE_CCS
	CCCSHandler::initialise(MAX_REPEATERS);
#endif
	CAudioUnit::initialise();
}

CDStarGatewayThread::~CDStarGatewayThread()
{
	CHeaderData::finalise();
	CG2Handler::finalise();
	CDExtraHandler::finalise();
	CDPlusHandler::finalise();
	CDCSHandler::finalise();
	CRepeaterHandler::finalise();
#ifdef USE_STARNET
	CStarNetHandler::finalise();
#endif
#ifdef USE_CCS
	CCCSHandler::finalise();
#endif
	CAudioUnit::finalise();
}

void* CDStarGatewayThread::Entry()
{
	// Truncate the old Links.log file
	std::string fullName = m_logDir + "/" + LINKS_BASE_NAME + ".log";
	if (!m_name.empty()) {
		fullName += fullName + m_name;
	}
	CUtils::truncateFile(fullName);
	CLog::logInfo("Truncating %s", fullName.c_str());

#ifdef USE_STARNET
	// Truncate the old StarNet.log file
	std::string fullName = m_logDir + "/" + STARNET_BASE_NAME + ".log";
	if (!m_name.empty()) {
		fullName += fullName + m_name;
	}
	CUtils::truncateFile(fullName);
	CLog::logInfo("Truncating %s", fullName.c_str());
#endif

	std::string dextraAddress = m_dextraEnabled ? m_gatewayAddress : LOOPBACK_ADDRESS;
	m_dextraPool = new CDExtraProtocolHandlerPool(DEXTRA_PORT, dextraAddress);
	// Allocate the incoming port
	CDExtraProtocolHandler* dextraHandler = m_dextraPool->getIncomingHandler();
	if(dextraHandler != NULL) {
		CDExtraHandler::setDExtraProtocolIncoming(dextraHandler);
		CDExtraHandler::setDExtraProtocolHandlerPool(m_dextraPool);
	}
	else {
		CLog::logError("Failed to allocate incoming DExtra handler\n");
	}
	

	std::string dplusAddress = m_dplusEnabled ? m_gatewayAddress : LOOPBACK_ADDRESS;
	m_dplusPool = new CDPlusProtocolHandlerPool(DPLUS_PORT, dplusAddress);
	CDPlusProtocolHandler* dplusHandler = m_dplusPool->getIncomingHandler();
	if(dplusHandler != NULL) {
		CDPlusHandler::setDPlusProtocolIncoming(dplusHandler);
		CDPlusHandler::setDPlusProtocolHandlerPool(m_dplusPool);
	} else {
		CLog::logError("Failed to allocate incoming DPlus handler\n");
	}

	std::string dcsAddress = m_dcsEnabled ? m_gatewayAddress : LOOPBACK_ADDRESS;
	m_dcsPool = new CDCSProtocolHandlerPool(DCS_PORT, dcsAddress);
	CDCSProtocolHandler* dcsHandler = m_dcsPool->getIncomingHandler();
	if(dcsHandler != NULL) {
		CDCSHandler::setDCSProtocolIncoming(dcsHandler);
		CDCSHandler::setDCSProtocolHandlerPool(m_dcsPool);
	} else {
		CLog::logError("Failed to allocate incoming DCS handler\n");
	}

	m_g2Handler = new CG2ProtocolHandler(G2_DV_PORT, m_gatewayAddress);
	bool ret = m_g2Handler->open();
	if (!ret) {
		CLog::logError("Could not open the G2 protocol handler");
		delete m_g2Handler;
		m_g2Handler = NULL;
	}

#if defined(ENABLE_NAT_TRAVERSAL)
	if(m_g2Handler != NULL) {
		m_natTraversal = new CNatTraversalHandler();
		m_natTraversal->setG2Handler(m_g2Handler);
	}
#endif

	// Wait here until we have the essentials to run
	while (!m_killed && (m_dextraPool == NULL || m_dplusPool == NULL || m_dcsPool == NULL || m_g2Handler == NULL || (m_icomRepeaterHandler == NULL && m_hbRepeaterHandler == NULL && m_dummyRepeaterHandler == NULL) || m_gatewayCallsign.empty()))
		::std::this_thread::sleep_for(std::chrono::milliseconds(500UL));		// 1/2 sec

	if (m_killed)
		return NULL;

	m_stopped = false;

	CLog::logInfo("Starting the ircDDB Gateway thread");

	CHeaderLogger* headerLogger = NULL;
	if (m_logEnabled) {
		m_statusTimer1.start();

		headerLogger = new CHeaderLogger(m_logDir, m_name);
		ret = headerLogger->open();
		if (!ret) {
			delete headerLogger;
			headerLogger = NULL;
		}
	}

	loadGateways();
	loadAllReflectors();

	CG2Handler::setG2ProtocolHandler(m_g2Handler);
	CG2Handler::setHeaderLogger(headerLogger);

	CDExtraHandler::setCallsign(m_gatewayCallsign);
	CDExtraHandler::setHeaderLogger(headerLogger);
	CDExtraHandler::setMaxDongles(m_dextraMaxDongles);

	CDPlusHandler::setCallsign(m_gatewayCallsign);
	CDPlusHandler::setDPlusLogin(m_dplusLogin);
	CDPlusHandler::setHeaderLogger(headerLogger);
	CDPlusHandler::setMaxDongles(m_dplusMaxDongles);
	if (m_dplusEnabled)
		CDPlusHandler::startAuthenticator(m_gatewayAddress, &m_cache);

	CDCSHandler::setGatewayType(m_gatewayType);
	CDCSHandler::setHeaderLogger(headerLogger);

	CRepeaterHandler::setLocalAddress(m_gatewayAddress);
	CRepeaterHandler::setG2Handler(m_g2Handler);

	if (m_irc != NULL)
		CRepeaterHandler::setIRC(m_irc);

	CRepeaterHandler::setCache(&m_cache);
	CRepeaterHandler::setGateway(m_gatewayCallsign);
	CRepeaterHandler::setLanguage(m_language);
	CRepeaterHandler::setDExtraEnabled(m_dextraEnabled);
	CRepeaterHandler::setDPlusEnabled(m_dplusEnabled);
	CRepeaterHandler::setDCSEnabled(m_dcsEnabled);
	CRepeaterHandler::setHeaderLogger(headerLogger);
	CRepeaterHandler::setAPRSWriter(m_aprsWriter);
	CRepeaterHandler::setInfoEnabled(m_infoEnabled);
	CRepeaterHandler::setEchoEnabled(m_echoEnabled);
	CRepeaterHandler::setDTMFEnabled(m_dtmfEnabled);
	if (m_whiteList != NULL) {
		CDExtraHandler::setWhiteList(m_whiteList);
		CDPlusHandler::setWhiteList(m_whiteList);
		CDCSHandler::setWhiteList(m_whiteList);
	}
	if (m_blackList != NULL) {
		CDExtraHandler::setBlackList(m_blackList);
		CDPlusHandler::setBlackList(m_blackList);
		CDCSHandler::setBlackList(m_blackList);
	}
	if (m_restrictList != NULL)
		CRepeaterHandler::setRestrictList(m_restrictList);

	CAudioUnit::setLanguage(m_language);

#ifdef USE_STARNET
	CStarNetHandler::setCache(&m_cache);
	CStarNetHandler::setGateway(m_gatewayCallsign);
	CStarNetHandler::setG2Handler(m_g2Handler);

	if (m_irc != NULL)
		CStarNetHandler::setIRC(m_irc);

	CStarNetHandler::setLogging(m_logEnabled, m_logDir);
#if defined(DEXTRA_LINK) || defined(DCS_LINK)
	CStarNetHandler::link();
#endif
#endif

	if (m_ddModeEnabled) {
		CDDHandler::initialise(MAX_DD_ROUTES, m_name);
		CDDHandler::setLogging(m_logEnabled, m_logDir);
		CDDHandler::setHeaderLogger(headerLogger);

		if (m_irc != NULL)
			CDDHandler::setIRC(m_irc);
	}

#ifdef USE_CCS
	std::string ccsAddress = m_ccsEnabled ? m_gatewayAddress : LOOPBACK_ADDRESS;
	CCCSHandler::setLocalAddress(ccsAddress);
	CCCSHandler::setHeaderLogger(headerLogger);
	CCCSHandler::setHost(m_ccsHost);
#endif

	if (m_remoteEnabled && !m_remotePassword.empty() && m_remotePort > 0U) {
		m_remote = new CRemoteHandler(m_remotePassword, m_remotePort, m_gatewayAddress);
		bool res = m_remote->open();
		if (!res) {
			delete m_remote;
			m_remote = NULL;
		}
	}

	CRepeaterHandler::startup();

	m_statusFileTimer.start();

#ifdef USE_CALLSIGN_SERVER
	CCallsignServer* server = NULL;
	if (m_dextraEnabled || m_dcsEnabled) {
		server = new CCallsignServer(m_gatewayCallsign, m_gatewayAddress, &m_cache);
		server->start();
	}
#endif

	auto timePoint = std::chrono::steady_clock::now();

	m_statusTimer2.start();

	try {
		while (!m_killed) {
			if (m_icomRepeaterHandler != NULL)
				processRepeater(m_icomRepeaterHandler);

			if (m_hbRepeaterHandler != NULL)
				processRepeater(m_hbRepeaterHandler);

			if (m_dummyRepeaterHandler != NULL)
				processRepeater(m_dummyRepeaterHandler);

			if (m_irc != NULL)
				processIrcDDB();

			processDExtra();
			processDPlus();
			processDCS();
			processG2();
#ifdef USE_CCS
			CCCSHandler::process();
#endif

			if (m_ddModeEnabled)
				processDD();

			if (m_remote != NULL)
				m_remote->process();

			unsigned long ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()- timePoint).count();
			timePoint = std::chrono::steady_clock::now();

			CRepeaterHandler::clock(ms);
			CG2Handler::clock(ms);
			CDExtraHandler::clock(ms);
			CDPlusHandler::clock(ms);
			CDCSHandler::clock(ms);
#ifdef USE_STARNET
			CStarNetHandler::clock(ms);
#endif
			CDDHandler::clock(ms);
#ifdef USE_CCS
	 		CCCSHandler::clock(ms);
#endif

			m_statusTimer2.clock(ms);

			m_statusFileTimer.clock(ms);
			if (m_statusFileTimer.hasExpired()) {
				readStatusFiles();
				m_statusFileTimer.start();
			}

			if (m_aprsWriter != NULL)
				m_aprsWriter->clock(ms);

			if (m_logEnabled) {
				m_statusTimer1.clock(ms);
				if (m_statusTimer1.hasExpired()) {
					bool ret1 = CDExtraHandler::stateChange();
					bool ret2 = CDPlusHandler::stateChange();
					bool ret3 = CDCSHandler::stateChange();
#ifdef USE_CCS
					bool ret4 = CCCSHandler::stateChange();
					if (ret1 || ret2 || ret3 || ret4)
						writeStatus();
#else
					if (ret1 || ret2 || ret3)
						writeStatus();
#endif
		
					m_statusTimer1.start();
				}
			}

			// wxLog::FlushActive();
			::std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PER_TIC_MS));
		}
	}
	catch (std::exception& e) {
		std::string message(e.what());
		CLog::logError("Exception raised in the main thread - \"%s\"", message.c_str());
	}
	catch (...) {
		CLog::logError("Unknown exception raised in the main thread");
	}

	CLog::logInfo("Stopping the ircDDB Gateway thread");

	// Unlink from all reflectors
	CDExtraHandler::unlink();
	CDPlusHandler::unlink();
	CDCSHandler::unlink();
#ifdef USE_CSS
	CCCSHandler::disconnect();
#endif

	if (m_ddModeEnabled)
		CDDHandler::finalise();

#ifdef USE_CALLSIGN_SERVER
	if (server != NULL)
		server->stop();
#endif

	m_dextraPool->close();
	delete m_dextraPool;

	m_dplusPool->close();
	delete m_dplusPool;

	m_dcsPool->close();
	delete m_dcsPool;

	m_g2Handler->close();
	delete m_g2Handler;

	if (m_irc != NULL) {
		m_irc->close();
		delete m_irc;
	}

	if (m_icomRepeaterHandler != NULL) {
		m_icomRepeaterHandler->close();
		delete m_icomRepeaterHandler;
	}

	if (m_hbRepeaterHandler != NULL) {
		m_hbRepeaterHandler->close();
		delete m_hbRepeaterHandler;
	}

	if (m_dummyRepeaterHandler != NULL) {
		m_dummyRepeaterHandler->close();
		delete m_dummyRepeaterHandler;
	}

	if (m_remote != NULL) {
		m_remote->close();
		delete m_remote;
	}

	if (headerLogger != NULL) {
		headerLogger->close();
		delete headerLogger;
	}
}

void CDStarGatewayThread::kill()
{
	m_killed = true;
}

void CDStarGatewayThread::setGateway(GATEWAY_TYPE gatewayType, const std::string& gatewayCallsign, const std::string& gatewayAddress)
{
	if (!m_stopped)
		return;

	m_gatewayType     = gatewayType;
	m_gatewayCallsign = gatewayCallsign;
	m_gatewayAddress  = gatewayAddress;
}

#ifdef USE_DRATS
void CDStarGatewayThread::addRepeater(const std::string& callsign, const std::string& band, const std::string& address, unsigned int port, HW_TYPE hwType, const std::string& reflector, bool atStartup, RECONNECT reconnect, bool dratsEnabled, double frequency, double offset, double range, double latitude, double longitude, double agl, const std::string& description1, const std::string& description2, const std::string& url, IRepeaterProtocolHandler* handler, unsigned char band1, unsigned char band2, unsigned char band3)
{
	CRepeaterHandler::add(callsign, band, address, port, hwType, reflector, atStartup, reconnect, dratsEnabled, frequency, offset, range, latitude, longitude, agl, description1, description2, url, handler, band1, band2, band3);
#else
void CDStarGatewayThread::addRepeater(const std::string& callsign, const std::string& band, const std::string& address, unsigned int port, HW_TYPE hwType, const std::string& reflector, bool atStartup, RECONNECT reconnect, double frequency, double offset, double range, double latitude, double longitude, double agl, const std::string& description1, const std::string& description2, const std::string& url, IRepeaterProtocolHandler* handler, unsigned char band1, unsigned char band2, unsigned char band3)
{
	CRepeaterHandler::add(callsign, band, address, port, hwType, reflector, atStartup, reconnect, frequency, offset, range, latitude, longitude, agl, description1, description2, url, handler, band1, band2, band3);
#endif

	std::string repeater = callsign;
	repeater.resize(LONG_CALLSIGN_LENGTH - 1U);
	repeater += band;

	// Add a fixed address and protocol for the local repeaters
	m_cache.updateRepeater(repeater, m_gatewayCallsign, "127.0.0.1", DP_LOOPBACK, true, true);

	CLog::logInfo("Adding %s to the cache as a local repeater", repeater.c_str());
}

#ifdef USE_STARNET
#if defined(DEXTRA_LINK) || defined(DCS_LINK)
void CDStarGatewayThread::addStarNet(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent, unsigned int userTimeout, unsigned int groupTimeout, STARNET_CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch, const std::string& reflector)
{
	CStarNetHandler::add(callsign, logoff, repeater, infoText, permanent, userTimeout, groupTimeout, callsignSwitch, txMsgSwitch, reflector);
}
#else
void CDStarGatewayThread::addStarNet(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent, unsigned int userTimeout, unsigned int groupTimeout, STARNET_CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch)
{
	CStarNetHandler::add(callsign, logoff, repeater, infoText, permanent, userTimeout, groupTimeout, callsignSwitch, txMsgSwitch);
}
#endif
#endif

void CDStarGatewayThread::setIcomRepeaterHandler(CIcomRepeaterProtocolHandler* handler)
{
	m_icomRepeaterHandler = handler;
}

void CDStarGatewayThread::setHBRepeaterHandler(CHBRepeaterProtocolHandler* handler)
{
	m_hbRepeaterHandler = handler;
}

void CDStarGatewayThread::setDummyRepeaterHandler(CDummyRepeaterProtocolHandler* handler)
{
	m_dummyRepeaterHandler = handler;
}

void CDStarGatewayThread::setIRC(CIRCDDB* irc)
{
	assert(irc != NULL);

	m_irc = irc;

	m_lastStatus = IS_DISCONNECTED;
}

void CDStarGatewayThread::setLanguage(TEXT_LANG language)
{
	m_language = language;
}

void CDStarGatewayThread::setDExtra(bool enabled, unsigned int maxDongles)
{
	if (enabled) {
		m_dextraEnabled    = true;
		m_dextraMaxDongles = maxDongles;
	} else {
		m_dextraEnabled    = false;
		m_dextraMaxDongles = 0U;
	}
}

void CDStarGatewayThread::setDPlus(bool enabled, unsigned int maxDongles, const std::string& login)
{
	if (enabled) {
		m_dplusEnabled    = true;
		m_dplusMaxDongles = maxDongles;
	} else {
		m_dplusEnabled    = false;
		m_dplusMaxDongles = 0U;
	}

	m_dplusLogin = login;
}

void CDStarGatewayThread::setDCS(bool enabled)
{
	m_dcsEnabled = enabled;
}

void CDStarGatewayThread::setXLX(bool enabled, const std::string& xlxHostsFileName)
{
	m_xlxEnabled 	 = enabled;
	m_xlxHostsFileName = xlxHostsFileName;
}

#ifdef USE_CCS
void CDStarGatewayThread::setCCS(bool enabled, const std::string& host)
{
	m_ccsEnabled = enabled;

	wxFileName fileName(wxFileName::GetHomeDir(), CCS_HOSTS_FILE_NAME);

	if (!fileName.IsFileReadable()) {
		CLog::logInfo(wxT("File %s not readable"), fileName.GetFullPath().c_str());
#if defined(__WINDOWS__)
		fileName.Assign(::wxGetCwd(), CCS_HOSTS_FILE_NAME);
#else
		fileName.Assign(wxT(m_dataDir), CCS_HOSTS_FILE_NAME);
#endif
		if (!fileName.IsFileReadable()) {
			CLog::logInfo(wxT("File %s not readable"), fileName.GetFullPath().c_str());
			m_ccsEnabled = false;
			return;
		}
	}

	CHostFile hostFile(fileName.GetFullPath(), true);

	m_ccsHost = hostFile.getAddress(host);
}
#endif

void CDStarGatewayThread::setLog(bool enabled)
{
	m_logEnabled = enabled;
}

void CDStarGatewayThread::setAPRSWriter(CAPRSWriter* writer)
{
	m_aprsWriter = writer;
}

void CDStarGatewayThread::setInfoEnabled(bool enabled)
{
	m_infoEnabled = enabled;
}

void CDStarGatewayThread::setEchoEnabled(bool enabled)
{
	m_echoEnabled = enabled;
}

void CDStarGatewayThread::setDTMFEnabled(bool enabled)
{
	m_dtmfEnabled = enabled;
}

void CDStarGatewayThread::setDDModeEnabled(bool enabled)
{
	m_ddModeEnabled = enabled;
}

void CDStarGatewayThread::setLocation(double latitude, double longitude)
{
	m_latitude  = latitude;
	m_longitude = longitude;
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

void CDStarGatewayThread::setWhiteList(CCallsignList* list)
{
	assert(list != NULL);

	m_whiteList = list;
}

void CDStarGatewayThread::setBlackList(CCallsignList* list)
{
	assert(list != NULL);

	m_blackList = list;
}

void CDStarGatewayThread::setRestrictList(CCallsignList* list)
{
	assert(list != NULL);

	m_restrictList = list;
}

void CDStarGatewayThread::processIrcDDB()
{
	// Once per second
	if (m_statusTimer2.hasExpired()) {
		int status = m_irc->getConnectionState();
		switch (status) {
			case 0:
			case 10:
				if (m_lastStatus != IS_DISCONNECTED) {
					CLog::logInfo("Disconnected from ircDDB");
					m_lastStatus = IS_DISCONNECTED;
				}
				break;
			case 7:
				if (m_lastStatus != IS_CONNECTED) {
					CLog::logInfo("Connected to ircDDB");
					m_lastStatus = IS_CONNECTED;
				}
				break;
			default:
				if (m_lastStatus != IS_CONNECTING) {
					CLog::logInfo("Connecting to ircDDB");
					m_lastStatus = IS_CONNECTING;
				}
				break;
		}

		m_statusTimer2.start();
	}

	// Process incoming ircDDB messages, updating the caches
	for (;;) {
		IRCDDB_RESPONSE_TYPE type = m_irc->getMessageType();

		switch (type) {
			case IDRT_USER: {
					std::string user, repeater, gateway, address, timestamp;
					bool res = m_irc->receiveUser(user, repeater, gateway, address, timestamp);
					if (!res)
						break;

					if (!address.empty()) {
						CLog::logInfo("USER: %s %s %s %s", user.c_str(), repeater.c_str(), gateway.c_str(), address.c_str());
						m_cache.updateUser(user, repeater, gateway, address, timestamp, DP_DEXTRA, false, false);
#if defined(ENABLE_NAT_TRAVERSAL)
						m_natTraversal->traverseNatG2(address);
#endif
					} else {
						CLog::logInfo("USER: %s NOT FOUND", user.c_str());
					}
				}
				break;

			case IDRT_REPEATER: {
					std::string repeater, gateway, address;
					bool res = m_irc->receiveRepeater(repeater, gateway, address);
					if (!res)
						break;

					CRepeaterHandler::resolveRepeater(repeater, gateway, address, DP_DEXTRA);
					if (!address.empty()) {
						CLog::logInfo("REPEATER: %s %s %s", repeater.c_str(), gateway.c_str(), address.c_str());
						m_cache.updateRepeater(repeater, gateway, address, DP_DEXTRA, false, false);
#if defined(ENABLE_NAT_TRAVERSAL)
						m_natTraversal->traverseNatG2(address);
#endif
					} else {
						CLog::logInfo("REPEATER: %s NOT FOUND", repeater.c_str());
					}
				}
				break;

			case IDRT_GATEWAY: {
					std::string gateway, address;
					bool res = m_irc->receiveGateway(gateway, address);
					if (!res)
						break;

					CDExtraHandler::gatewayUpdate(gateway, address);
					CDPlusHandler::gatewayUpdate(gateway, address);
					if (!address.empty()) {
						CLog::logInfo("GATEWAY: %s %s", gateway.c_str(), address.c_str());
						m_cache.updateGateway(gateway, address, DP_DEXTRA, false, false);
#if defined(ENABLE_NAT_TRAVERSAL)						
						m_natTraversal->traverseNatG2(address);
#endif
					} else {
						CLog::logInfo("GATEWAY: %s NOT FOUND", gateway.c_str());
					}
				}
				break;

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processRepeater(IRepeaterProtocolHandler* handler)
{
	for (;;) {
		REPEATER_TYPE type = handler->read();

		switch (type) {
			case RT_POLL: {
					CPollData* poll = handler->readPoll();
					if (poll != NULL) {
						CRepeaterHandler* handler = CRepeaterHandler::findRepeater(*poll);
						if (handler != NULL)
							handler->processRepeater(*poll);
						else
							CRepeaterHandler::pollAllIcom(*poll);

						delete poll;
					}
				}
				break;

			case RT_HEARD: {
					CHeardData* heard = handler->readHeard();
					if (heard != NULL) {
						std::string user = heard->getUser();
						std::string repeater = heard->getRepeater();

						// Internal controller heard have identical user and repeater values
						if (repeater != user) {
							CRepeaterHandler* handler = CRepeaterHandler::findDVRepeater(repeater);
							if (handler == NULL)
								CLog::logInfo("Heard received from unknown repeater, %s", repeater.c_str());
							else
								handler->processRepeater(*heard);

							delete heard;
						}
					}
				}
				break;

			case RT_HEADER: {
					CHeaderData* header = handler->readHeader();
					if (header != NULL) {
						// CLog::logInfo(wxT("Repeater header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str(), header->getFlag1(), header->getFlag2(), header->getFlag3());

						CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(*header);
						if (repeater == NULL)
							CLog::logInfo("Header received from unknown repeater, %s", header->getRptCall1().c_str());
						else
							repeater->processRepeater(*header);

						delete header;
					}
				}
				break;

			case RT_AMBE: {
					CAMBEData* data = handler->readAMBE();
					if (data != NULL) {
						CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(*data, false);
						if (repeater != NULL)
							repeater->processRepeater(*data);

						delete data;
					}
				}
				break;

			case RT_BUSY_HEADER: {
					CHeaderData* header = handler->readBusyHeader();
					if (header != NULL) {
						// CLog::logInfo(wxT("Repeater busy header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str(), header->getFlag1(), header->getFlag2(), header->getFlag3());

						CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(*header);
						if (repeater == NULL)
							CLog::logInfo("Busy header received from unknown repeater, %s", header->getRptCall1().c_str());
						else
							repeater->processBusy(*header);

						delete header;
					}
				}
				break;

			case RT_BUSY_AMBE: {
					CAMBEData* data = handler->readBusyAMBE();
					if (data != NULL) {
						CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(*data, true);
						if (repeater != NULL)
							repeater->processBusy(*data);

						delete data;
					}
				}
				break;

			case RT_DD: {
					CDDData* data = handler->readDD();
					if (data != NULL) {
						// CLog::logInfo(wxT("DD header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X"), data->getMyCall1().c_str(), data->getMyCall2().c_str(), data->getYourCall().c_str(), data->getRptCall1().c_str(), data->getRptCall2().c_str(), data->getFlag1(), data->getFlag2(), data->getFlag3());

						CRepeaterHandler* repeater = CRepeaterHandler::findDDRepeater();
						if (repeater == NULL)
							CLog::logInfo("DD data received from unknown DD repeater, %s", data->getRptCall1().c_str());
						else
							repeater->processRepeater(*data);

						delete data;
					}
				}
				break;

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processDExtra()
{
	for (;;) {
		DEXTRA_TYPE type = m_dextraPool->read();

		switch (type) {
			case DE_POLL: {
					CPollData* poll = m_dextraPool->readPoll();
					if (poll != NULL) {
						CDExtraHandler::process(*poll);
						delete poll;
					}
				}
				break;

			case DE_CONNECT: {
					CConnectData* connect = m_dextraPool->readConnect();
					if (connect != NULL) {
						CDExtraHandler::process(*connect);
						delete connect;
					}
				}
				break;

			case DE_HEADER: {
					CHeaderData* header = m_dextraPool->readHeader();
					if (header != NULL) {
						// CLog::logInfo(wxT("DExtra header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str());
						CDExtraHandler::process(*header);
						delete header;
					}
				}
				break;

			case DE_AMBE: {
					CAMBEData* data = m_dextraPool->readAMBE();
					if (data != NULL) {
						CDExtraHandler::process(*data);
						delete data;
					}
				}
				break;

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processDPlus()
{
	for (;;) {
		DPLUS_TYPE type = m_dplusPool->read();

		switch (type) {
			case DP_POLL: {
					CPollData* poll = m_dplusPool->readPoll();
					if (poll != NULL) {
						CDPlusHandler::process(*poll);
						delete poll;
					}
				}
				break;

			case DP_CONNECT: {
					CConnectData* connect = m_dplusPool->readConnect();
					if (connect != NULL) {
						CDPlusHandler::process(*connect);
						delete connect;
					}
				}
				break;

			case DP_HEADER: {
					CHeaderData* header = m_dplusPool->readHeader();
					if (header != NULL) {
						// CLog::logInfo(wxT("D-Plus header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str());
						CDPlusHandler::process(*header);
						delete header;
					}
				}
				break;

			case DP_AMBE: {
					CAMBEData* data = m_dplusPool->readAMBE();
					if (data != NULL) {
						CDPlusHandler::process(*data);
						delete data;
					}
				}
				break;

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processDCS()
{
	for (;;) {
		DCS_TYPE type = m_dcsPool->read();

		switch (type) {
			case DC_POLL: {
					CPollData* poll = m_dcsPool->readPoll();
					if (poll != NULL) {
						CDCSHandler::process(*poll);
						delete poll;
					}
				}
				break;

			case DC_CONNECT: {
					CConnectData* connect = m_dcsPool->readConnect();
					if (connect != NULL) {
						CDCSHandler::process(*connect);
						delete connect;
					}
				}
				break;

			case DC_DATA: {
					CAMBEData* data = m_dcsPool->readData();
					if (data != NULL) {
						// CLog::logInfo(wxT("DCS header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str());
						CDCSHandler::process(*data);
						delete data;
					}
				}
				break;

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processG2()
{
	for (;;) {
		G2_TYPE type = m_g2Handler->read();

		switch (type) {
			case GT_HEADER: {
					CHeaderData* header = m_g2Handler->readHeader();
					if (header != NULL) {
						// CLog::logInfo(wxT("G2 header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X"), header->getMyCall1().c_str(), header->getMyCall2().c_str(), header->getYourCall().c_str(), header->getRptCall1().c_str(), header->getRptCall2().c_str(), header->getFlag1(), header->getFlag2(), header->getFlag3());
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

			default:
				return;
		}
	}
}

void CDStarGatewayThread::processDD()
{
	for (;;) {
		CDDData* data = CDDHandler::read();
		if (data == NULL)
			return;

		// CLog::logInfo(wxT("DD header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X"), data->getMyCall1().c_str(), data->getMyCall2().c_str(), data->getYourCall().c_str(), data->getRptCall1().c_str(), data->getRptCall2().c_str(), data->getFlag1(), data->getFlag2(), data->getFlag3());

		delete data;
	}
}

void CDStarGatewayThread::loadGateways()
{
	std::string fileName = m_dataDir + "/" + GATEWAY_HOSTS_FILE_NAME;
	loadReflectors(fileName, DP_DEXTRA);
}

void CDStarGatewayThread::loadAllReflectors()
{
	if (m_xlxEnabled) {
		loadReflectors(m_xlxHostsFileName, DP_DCS);
	}
	
	if (m_dplusEnabled) {
		std::string fileName = m_dataDir + "/" + DPLUS_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DPLUS);
	}

	if (m_dextraEnabled) {
		std::string fileName = m_dataDir + "/" + DCS_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DEXTRA);
	}

	if (m_dcsEnabled) {
		std::string fileName = m_dataDir + "/" + DCS_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DCS);
	}
}

void CDStarGatewayThread::loadReflectors(std::string hostFileName, DSTAR_PROTOCOL proto)
{
	unsigned int count = 0U;

	CHostFile hostFile(hostFileName, false);
	for (unsigned int i = 0U; i < hostFile.getCount(); i++) {
		std::string reflector = hostFile.getName(i);
		in_addr address    = CUDPReaderWriter::lookup(hostFile.getAddress(i));
		bool lock          = hostFile.getLock(i);

		if (address.s_addr != INADDR_NONE) {
			unsigned char* ucp = (unsigned char*)&address;

			std::string addrText;
			addrText = CStringUtils::string_format("%u.%u.%u.%u", ucp[0U] & 0xFFU, ucp[1U] & 0xFFU, ucp[2U] & 0xFFU, ucp[3U] & 0xFFU);

			if (lock)
				CLog::logInfo("Locking %s to %s", reflector.c_str(), addrText.c_str());

			reflector.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
			reflector += "G";
			m_cache.updateGateway(reflector, addrText, proto, lock, true);

			count++;
		}
	}

	std::string protoString;
	switch (proto)
	{
	case DP_DEXTRA:
		protoString =  "DExtra";
		break;
	case DP_DCS:
		protoString = "DCS";
		break;
	case DP_DPLUS:
		protoString = "DPlus";
		break;
	default:
		// ???
		break;
	}

	CLog::logInfo("Loaded %u of %u %s hosts from %s", count, hostFile.getCount(), protoString.c_str() , hostFileName.c_str());
}

void CDStarGatewayThread::writeStatus()
{
	std::string fullName = LINKS_BASE_NAME;

	if (!m_name.empty()) {
		fullName += "_" + m_name;
	}

	fullName = m_logDir + "/" + LINKS_BASE_NAME + ".log";

	std::ofstream file;
	file.open(fullName, std::fstream::trunc);
	if (!file.is_open()) {
		CLog::logError("Unable to open %s for writing", fullName.c_str());
		return;
	}

	CDExtraHandler::writeStatus(file);
	CDPlusHandler::writeStatus(file);
	CDCSHandler::writeStatus(file);
#ifdef USE_CCS
	CCCSHandler::writeStatus(file);
#endif

	file.close();
}

CIRCDDBGatewayStatusData* CDStarGatewayThread::getStatus() const
{
	bool aprsStatus = false;
	if (m_aprsWriter != NULL)
		aprsStatus = m_aprsWriter->isConnected();

	CIRCDDBGatewayStatusData* status = new CIRCDDBGatewayStatusData(m_lastStatus, aprsStatus);

	for (unsigned int i = 0U; i < 4U; i++) {
		std::string callsign, linkCallsign;
		LINK_STATUS linkStatus;
		bool ret = CRepeaterHandler::getRepeater(i, callsign, linkStatus, linkCallsign);
		if (ret) {
			std::string incoming1 = CDExtraHandler::getIncoming(callsign);
			std::string incoming2 = CDCSHandler::getIncoming(callsign);
#if USE_CCS
			std::string incoming3 = CCCSHandler::getIncoming(callsign);
#endif

			std::string incoming;
			if (!incoming1.empty()) {
				incoming = incoming1 + " ";
			}
			if (!incoming2.empty()) {
				incoming = incoming2 + " ";
			}
#if USE_CCS
			if (!incoming3.empty()) {
				incoming = incoming3 + " ";
			}
#endif

			status->setRepeater(i, callsign, linkStatus, linkCallsign, incoming);
		}
	}

	std::string dongles;
	dongles += CDExtraHandler::getDongles();
	dongles += CDPlusHandler::getDongles();
	status->setDongles(dongles);

	return status;
}

void CDStarGatewayThread::readStatusFiles()
{
	readStatusFile(STATUS1_FILE_NAME, 0U, m_status1);
	readStatusFile(STATUS2_FILE_NAME, 1U, m_status2);
	readStatusFile(STATUS3_FILE_NAME, 2U, m_status3);
	readStatusFile(STATUS4_FILE_NAME, 3U, m_status4);
	readStatusFile(STATUS5_FILE_NAME, 4U, m_status5);
}

void CDStarGatewayThread::readStatusFile(const std::string& filename, unsigned int n, std::string& var)
{
	std::string fullFileName = m_dataDir + "/" + filename;
	std::string text;
	std::ifstream file;

	file.open(fullFileName);
	if(file.is_open()) {
		std::getline(file, text);
		file.close();
	}

	if(var != text) {
		CLog::logInfo("Status %u message set to \"%s\"", n + 1U, text.c_str());
		CStatusData statusData(text, n);
		CRepeaterHandler::writeStatus(statusData);
		var = text;
	}
}
