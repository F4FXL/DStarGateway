/*
 *   Copyright (C) 2010-2015,2018 by Jonathan Naylor G4KLX
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

#include <cstring>
#include <boost/algorithm/string.hpp>

#include "RepeaterHandler.h"
#include "DExtraHandler.h"
#include "DPlusHandler.h"
#include "DStarDefines.h"
#include "DCSHandler.h"
#include "HeaderData.h"
#include "DDHandler.h"
#include "AMBEData.h"
#include "Utils.h"
#include "Log.h"
#include "StringUtils.h"


const unsigned int  ETHERNET_ADDRESS_LENGTH = 6U;

const unsigned char ETHERNET_BROADCAST_ADDRESS[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
// Multicast address '01:00:5E:00:00:01' - IP: '224.0.0.1' (to all)
const unsigned char TOALL_MULTICAST_ADDRESS[] = {0x01U, 0x00U, 0x5EU, 0x00U, 0x00U, 0x01U};
// Multicast address '01:00:5E:00:00:23' - IP: '224.0.0.35' (DX-Cluster)
const unsigned char DX_MULTICAST_ADDRESS[] = {0x01U, 0x00U, 0x5EU, 0x00U, 0x00U, 0x23U};

unsigned int              CRepeaterHandler::m_maxRepeaters = 0U;
CRepeaterHandler**        CRepeaterHandler::m_repeaters = NULL;

std::string                  CRepeaterHandler::m_localAddress;
CG2ProtocolHandlerPool*       CRepeaterHandler::m_g2HandlerPool = NULL;
CIRCDDB*                  CRepeaterHandler::m_irc = NULL;
CCacheManager*            CRepeaterHandler::m_cache = NULL;
std::string                  CRepeaterHandler::m_gateway;
TEXT_LANG                 CRepeaterHandler::m_language = TL_ENGLISH_UK;
bool                      CRepeaterHandler::m_dextraEnabled = true;
bool                      CRepeaterHandler::m_dplusEnabled = false;
bool                      CRepeaterHandler::m_dcsEnabled = true;
bool                      CRepeaterHandler::m_infoEnabled = true;
bool                      CRepeaterHandler::m_echoEnabled = true;
bool                      CRepeaterHandler::m_dtmfEnabled = true;

CHeaderLogger*            CRepeaterHandler::m_headerLogger = NULL;

CAPRSHandler*              CRepeaterHandler::m_outgoingAprsHandler  = NULL; //handles APRS/DPRS frames coming from radio to network
CAPRSHandler*              CRepeaterHandler::m_incomingAprsHandler  = NULL; //handles APRS/DPRS frames coming from network to radio

CCallsignList*            CRepeaterHandler::m_restrictList = NULL;

CRepeaterHandler::CRepeaterHandler(const std::string& callsign, const std::string& band, const std::string& address, unsigned int port, HW_TYPE hwType, const std::string& reflector, bool atStartup, RECONNECT reconnect, bool dratsEnabled, double frequency, double offset, double range, double latitude, double longitude, double agl, const std::string& description1, const std::string& description2, const std::string& url, IRepeaterProtocolHandler* handler, unsigned char band1, unsigned char band2, unsigned char band3) :

m_index(0x00U),
m_rptCallsign(),
m_gwyCallsign(),
m_band(' '),
m_address(),
m_port(port),
m_hwType(hwType),
m_repeaterHandler(handler),
m_frequency(frequency),
m_offset(offset),
m_range(range),
m_latitude(latitude),
m_longitude(longitude),
m_agl(agl),
m_description1(description1),
m_description2(description2),
m_url(url),
m_band1(band1),
m_band2(band2),
m_band3(band3),
m_repeaterId(0x00U),
m_busyId(0x00U),
m_watchdogTimer(1000U, REPEATER_TIMEOUT),
m_ddMode(false),
m_ddCallsign(),
m_queryTimer(1000U, 5U),		// 5 seconds
m_myCall1(),
m_myCall2(),
m_yourCall(),
m_rptCall1(),
m_rptCall2(),
m_flag1(0x00U),
m_flag2(0x00U),
m_flag3(0x00U),
m_restricted(false),
m_fastData(false),
m_frames(0U),
m_silence(0U),
m_errors(0U),
m_textCollector(),
m_text(),
m_xBandRptr(NULL),
#ifdef USE_STARNET
m_starNet(NULL),
#endif
m_g2Status(G2_NONE),
m_g2User(),
m_g2Repeater(),
m_g2Gateway(),
m_g2Header(NULL),
m_g2Address(),
m_linkStatus(LS_NONE),
m_linkRepeater(),
m_linkGateway(),
m_linkReconnect(reconnect),
m_linkAtStartup(atStartup),
m_linkStartup(reflector),
m_linkReconnectTimer(1000U),
m_linkRelink(false),
m_echo(NULL),
m_infoAudio(NULL),
m_infoNeeded(false),
#ifdef USE_ANNOUNCE
m_msgAudio(NULL),
m_msgNeeded(false),
m_wxAudio(NULL),
m_wxNeeded(false),
#endif
m_version(NULL),
m_drats(NULL),
m_dtmf(),
m_pollTimer(1000U, 900U),			// 15 minutes
#ifdef USE_CSS
m_ccsHandler(NULL),
#endif
m_lastReflector(),
m_heardUser(),
m_heardRepeater(),
m_heardTimer(1000U, 0U, 100U)		// 100ms
{
	assert(!callsign.empty());
	assert(port > 0U);
	assert(handler != NULL);

	m_ddMode = band.length() > 1U;

	m_band = band[0U];

	m_rptCallsign = callsign;
	m_rptCallsign += "        ";
	m_rptCallsign = m_rptCallsign.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	m_rptCallsign += band;
	m_rptCallsign = m_rptCallsign.substr(0, LONG_CALLSIGN_LENGTH);

	m_gwyCallsign = callsign;
	m_gwyCallsign += "        ";
	m_gwyCallsign = m_gwyCallsign.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	m_gwyCallsign += "G";

	m_address.s_addr = ::inet_addr(address.c_str());

	m_pollTimer.start();

	switch (m_linkReconnect) {
		case RECONNECT_5MINS:
			m_linkReconnectTimer.start(5U * 60U);
			break;
		case RECONNECT_10MINS:
			m_linkReconnectTimer.start(10U * 60U);
			break;
		case RECONNECT_15MINS:
			m_linkReconnectTimer.start(15U * 60U);
			break;
		case RECONNECT_20MINS:
			m_linkReconnectTimer.start(20U * 60U);
			break;
		case RECONNECT_25MINS:
			m_linkReconnectTimer.start(25U * 60U);
			break;
		case RECONNECT_30MINS:
			m_linkReconnectTimer.start(30U * 60U);
			break;
		case RECONNECT_60MINS:
			m_linkReconnectTimer.start(60U * 60U);
			break;
		case RECONNECT_90MINS:
			m_linkReconnectTimer.start(90U * 60U);
			break;
		case RECONNECT_120MINS:
			m_linkReconnectTimer.start(120U * 60U);
			break;
		case RECONNECT_180MINS:
			m_linkReconnectTimer.start(180U * 60U);
			break;
		default:
			break;
	}

#ifdef USE_ANNOUNCE
	wxFileName messageFile;
	messageFile.SetPath(::wxGetHomeDir());
	messageFile.SetName("message"));
	messageFile.SetExt("dvtool"));

	wxFileName weatherFile;
	weatherFile.SetPath(::wxGetHomeDir());
	weatherFile.SetName("weather"));
	weatherFile.SetExt("dvtool"));

	m_msgAudio  = new CAnnouncementUnit(this, callsign, messageFile.GetFullPath(), "MSG"));
	m_wxAudio   = new CAnnouncementUnit(this, callsign, weatherFile.GetFullPath(), "WX"));
#endif

	m_echo      = new CEchoUnit(this, callsign);
	m_infoAudio = new CAudioUnit(this, callsign);
	m_version   = new CVersionUnit(this, callsign);
	m_aprsUnit = new CAPRSUnit(this);


	if (dratsEnabled) {
		m_drats = new CDRATSServer(m_localAddress, port, callsign, this);
		bool ret = m_drats->open();
		if (!ret) {
			delete m_drats;
			m_drats = NULL;
		}
	}
}

CRepeaterHandler::~CRepeaterHandler()
{
	delete m_echo;
	delete m_infoAudio;
#ifdef USE_ANNOUNCE
	delete m_msgAudio;
	delete m_wxAudio;
#endif
	delete m_version;

	if (m_drats != NULL)
		m_drats->close();
}

void CRepeaterHandler::initialise(unsigned int maxRepeaters)
{
	assert(maxRepeaters > 0U);

	m_maxRepeaters = maxRepeaters;

	m_repeaters = new CRepeaterHandler*[m_maxRepeaters];
	for (unsigned int i = 0U; i < m_maxRepeaters; i++)
		m_repeaters[i] = NULL;
}

void CRepeaterHandler::setIndex(unsigned int index)
{
	m_index = index;
}

void CRepeaterHandler::add(const std::string& callsign, const std::string& band, const std::string& address, unsigned int port, HW_TYPE hwType, const std::string& reflector, bool atStartup, RECONNECT reconnect, bool dratsEnabled, double frequency, double offset, double range, double latitude, double longitude, double agl, const std::string& description1, const std::string& description2, const std::string& url, IRepeaterProtocolHandler* handler, unsigned char band1, unsigned char band2, unsigned char band3)
{
	assert(!callsign.empty());
	assert(port > 0U);
	assert(handler != NULL);


	CRepeaterHandler* repeater = new CRepeaterHandler(callsign, band, address, port, hwType, reflector, atStartup, reconnect, dratsEnabled, frequency, offset, range, latitude, longitude, agl, description1, description2, url, handler, band1, band2, band3);

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] == NULL) {
			repeater->setIndex(i);
			m_repeaters[i] = repeater;
			return;
		}
	}

	CLog::logError("Cannot add repeater with callsign %s, no space", callsign.c_str());

	delete repeater;
}

void CRepeaterHandler::setG2HandlerPool(CG2ProtocolHandlerPool* handler)
{
	assert(handler != NULL);

	m_g2HandlerPool = handler;
}

void CRepeaterHandler::setCache(CCacheManager* cache)
{
	assert(cache != NULL);

	m_cache = cache;
}

void CRepeaterHandler::setIRC(CIRCDDB* irc)
{
	assert(irc != NULL);

	m_irc = irc;
}

void CRepeaterHandler::setGateway(const std::string& gateway)
{
	m_gateway = gateway;
}

void CRepeaterHandler::setLanguage(TEXT_LANG language)
{
	m_language = language;
}

void CRepeaterHandler::setDExtraEnabled(bool enabled)
{
	m_dextraEnabled = enabled;
}

void CRepeaterHandler::setDPlusEnabled(bool enabled)
{
	m_dplusEnabled = enabled;
}

void CRepeaterHandler::setDCSEnabled(bool enabled)
{
	m_dcsEnabled = enabled;
}

void CRepeaterHandler::setInfoEnabled(bool enabled)
{
	m_infoEnabled = enabled;
}

void CRepeaterHandler::setEchoEnabled(bool enabled)
{
	m_echoEnabled = enabled;
}

void CRepeaterHandler::setDTMFEnabled(bool enabled)
{
	m_dtmfEnabled = enabled;
}

void CRepeaterHandler::setHeaderLogger(CHeaderLogger* logger)
{
	m_headerLogger = logger;
}

void CRepeaterHandler::setAPRSHandlers(CAPRSHandler* outgoingAprsHandler, CAPRSHandler* incomingAprsHandler)
{
	m_outgoingAprsHandler = outgoingAprsHandler;
	m_incomingAprsHandler = incomingAprsHandler;
}

void CRepeaterHandler::setLocalAddress(const std::string& address)
{
	m_localAddress = address;
}

void CRepeaterHandler::setRestrictList(CCallsignList* list)
{
	assert(list != NULL);

	m_restrictList = list;
}

bool CRepeaterHandler::getRepeater(unsigned int n, std::string& callsign, LINK_STATUS& linkStatus, std::string& linkCallsign)
{
	if (n >= m_maxRepeaters)
		return false;

	if (m_repeaters[n] == NULL)
		return false;

	callsign     = m_repeaters[n]->m_rptCallsign;
	linkStatus   = m_repeaters[n]->m_linkStatus;
	linkCallsign = m_repeaters[n]->m_linkRepeater;

	return true;
}

void CRepeaterHandler::resolveUser(const std::string &user, const std::string& repeater, const std::string& gateway, const std::string &address)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] != NULL)
			m_repeaters[i]->resolveUserInt(user, repeater, gateway, address);
	}
}

void CRepeaterHandler::resolveRepeater(const std::string& repeater, const std::string& gateway, const std::string &address, DSTAR_PROTOCOL protocol)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] != NULL)
			m_repeaters[i]->resolveRepeaterInt(repeater, gateway, address, protocol);
	}
}

void CRepeaterHandler::startup()
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] != NULL)
			m_repeaters[i]->startupInt();
	}
}

void CRepeaterHandler::clock(unsigned int ms)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] != NULL)
			m_repeaters[i]->clockInt(ms);
	}
}

void CRepeaterHandler::finalise()
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		delete m_repeaters[i];
		m_repeaters[i] = NULL;
	}

	delete[] m_repeaters;
}

CRepeaterHandler* CRepeaterHandler::findDVRepeater(const CHeaderData& header)
{
	std::string rpt1 = header.getRptCall1();
	in_addr address = header.getYourAddress();

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (!repeater->m_ddMode && repeater->m_address.s_addr == address.s_addr && repeater->m_rptCallsign == rpt1)
				return repeater;
		}
	}

	return NULL;
}

CRepeaterHandler* CRepeaterHandler::findDVRepeater(const CAMBEData& data, bool busy)
{
	unsigned int id = data.getId();

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (!busy && !repeater->m_ddMode && repeater->m_repeaterId == id)
				return repeater;
			if (busy && !repeater->m_ddMode && repeater->m_busyId == id)
				return repeater;
		}
	}

	return NULL;
}

CRepeaterHandler* CRepeaterHandler::findDVRepeater(const std::string& callsign)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (!repeater->m_ddMode && repeater->m_rptCallsign == callsign)
				return repeater;
		}
	}

	return NULL;
}

CRepeaterHandler* CRepeaterHandler::findRepeater(const CPollData& data)
{
	in_addr   address = data.getYourAddress();
	unsigned int port = data.getYourPort();

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (repeater->m_address.s_addr == address.s_addr && repeater->m_port == port)
				return repeater;
		}
	}

	return NULL;
}

CRepeaterHandler* CRepeaterHandler::findDDRepeater(const CDDData& data)
{
	std::string rpt1 = data.getRptCall1();

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (repeater->m_ddMode && repeater->m_rptCallsign == rpt1)
				return repeater;
		}
	}

	return NULL;
}

CRepeaterHandler* CRepeaterHandler::findDDRepeater()
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL) {
			if (repeater->m_ddMode)
				return repeater;
		}
	}

	return NULL;
}

std::vector<std::string> CRepeaterHandler::listDVRepeaters()
{
	std::vector<std::string> repeaters;

	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL && !repeater->m_ddMode)
			repeaters.push_back(repeater->m_rptCallsign);
	}

	return repeaters;
}

void CRepeaterHandler::pollAllIcom(CPollData& data)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		CRepeaterHandler* repeater = m_repeaters[i];
		if (repeater != NULL && repeater->m_hwType == HW_ICOM)
			repeater->processRepeater(data);
	}
}

CRemoteRepeaterData* CRepeaterHandler::getInfo() const
{
	return new CRemoteRepeaterData(m_rptCallsign, m_linkReconnect, m_linkStartup);
}

void CRepeaterHandler::processRepeater(CHeaderData& header)
{
	unsigned int id = header.getId();

	// Stop duplicate headers
	if (id == m_repeaterId)
		return;

	// Save the header fields
	m_myCall1  = header.getMyCall1();
	m_myCall2  = header.getMyCall2();
	m_yourCall = header.getYourCall();
	m_rptCall1 = header.getRptCall1();
	m_rptCall2 = header.getRptCall2();
	m_flag1    = header.getFlag1();
	m_flag2    = header.getFlag2();
	m_flag3    = header.getFlag3();

	if (m_hwType == HW_ICOM) {
		unsigned char band1 = header.getBand1();
		unsigned char band2 = header.getBand2();
		unsigned char band3 = header.getBand3();

		if (m_band1 != band1 || m_band2 != band2 || m_band3 != band3) {
			m_band1 = band1;
			m_band2 = band2;
			m_band3 = band3;
			CLog::logInfo("Repeater %s registered with bands %u %u %u", m_rptCall1.c_str(), m_band1, m_band2, m_band3);
		}
	}

	if (m_flag1 == 0x01) {
		CLog::logInfo("Received a busy message from repeater %s", m_rptCall1.c_str());
		return;
	}

	if (!m_heardUser.empty() && m_myCall1 != m_heardUser && m_irc != NULL)
		m_irc->sendHeard(m_heardUser, "    ", "        ", m_heardRepeater, "        ", 0x00U, 0x00U, 0x00U);

#ifdef USE_CCS
	// Inform CCS
	m_ccsHandler->writeHeard(header);
	m_ccsHandler->writeHeader(header);
#endif

	// The Icom heard timer
	m_heardTimer.stop();

	if (m_drats != NULL)
		m_drats->writeHeader(header);

	// Reset the statistics
	m_frames  = 0U;
	m_silence = 0U;
	m_errors  = 0U;

	// Assume voice mode
	m_fastData = false;

	// An RF header resets the reconnect timer
	m_linkReconnectTimer.start();

	// Incoming links get everything
	sendToIncoming(header);

	// Reset the slow data text collector
	m_textCollector.reset();
	m_text.clear();

	// Reset the APRS Writer if it's enabled
	if (m_outgoingAprsHandler != NULL)
		m_outgoingAprsHandler->writeHeader(m_rptCallsign, header);

	// Write to Header.log if it's enabled
	if (m_headerLogger != NULL)
		m_headerLogger->write("Repeater", header);

	// Reset the DTMF decoder
	m_dtmf.reset();

	// Reset the info, echo and version commands if they're running
	m_infoAudio->cancel();
#ifdef USE_ANNOUNCE
	m_msgAudio->cancel();
	m_wxAudio->cancel();
#endif
	m_echo->cancel();
	m_version->cancel();

	// A new header resets fields and G2 routing status
	m_repeaterId = id;
	m_busyId     = 0x00U;
	m_watchdogTimer.start();

	m_xBandRptr = NULL;
#ifdef USE_STARNET
	m_starNet   = NULL;
#endif

	// If we're querying for a user or repeater, kill the query timer
	if (m_g2Status == G2_USER || m_g2Status == G2_REPEATER)
		m_queryTimer.stop();

	delete m_g2Header;
	m_g2Header = NULL;
	m_g2Status = G2_NONE;
	m_g2User.clear();
	m_g2Repeater.clear();
	m_g2Gateway.clear();

	// Check if this user is restricted
	m_restricted = false;
	if (m_restrictList != NULL) {
		bool res = m_restrictList->isInList(m_myCall1);
		if (res)
			m_restricted = true;
	}

	// Reject silly RPT2 values
	if (m_rptCall2 == (m_rptCallsign) || m_rptCall2 == ("        "))
		return;

	// Do cross-band routing if RPT2 is not one of the gateway callsigns
	if (m_rptCall2 != (m_gwyCallsign) && m_rptCall2 != m_gateway) {
		CRepeaterHandler* repeater = findDVRepeater(m_rptCall2);
		if (repeater != NULL) {
			CLog::logInfo("Cross-band routing by %s from %s to %s", m_myCall1.c_str(), m_rptCallsign.c_str(), m_rptCall2.c_str());
			m_xBandRptr = repeater;
			m_xBandRptr->process(header, DIR_INCOMING, AS_XBAND);
			m_g2Status = G2_XBAND;
		} else {
			// Keep the transmission local
			CLog::logInfo("Invalid cross-band route by %s from %s to %s", m_myCall1.c_str(), m_rptCallsign.c_str(), m_rptCall2.c_str());
			m_g2Status = G2_LOCAL;
		}
		return;
	}

#ifdef USE_STARNET
	m_starNet = CStarNetHandler::findStarNet(header);
	if (m_starNet != NULL && !m_restricted) {
		CLog::logInfo("StarNet routing by %s to %s", m_myCall1.c_str(), m_yourCall.c_str());
		m_starNet->process(header);
		m_g2Status = G2_STARNET;
		return;
	}
#endif

	// Reject simple cases
	if (m_yourCall.substr(0,4) == ("CQCQ")) {
		sendToOutgoing(header);
		return;
	}

	// Handle the Echo command
	if (m_echoEnabled && m_yourCall == ("       E")) {
		m_g2Status = G2_ECHO;
		m_echo->writeHeader(header);
		return;
	}

	// Handle the Info command
	if (m_infoEnabled && m_yourCall == ("       I")) {
		m_g2Status = G2_LOCAL;
		m_infoNeeded = true;
		return;
	}

#ifdef USE_ANNOUNCE
	// Handle the MSG command
	if (m_infoEnabled && m_yourCall == ("       M")) {
		m_g2Status = G2_LOCAL;
		m_msgNeeded = true;
		return;
	}

	// Handle the WX command
	if (m_infoEnabled && m_yourCall == ("       W")) {
		m_g2Status = G2_LOCAL;
		m_wxNeeded = true;
		return;
	}
#endif

	// Handle the Version command
	if (m_infoEnabled && m_yourCall == ("       V")) {
		m_g2Status = G2_VERSION;
		sendToOutgoing(header);
		return;
	}

	if (m_restricted) {
		sendToOutgoing(header);
		return;
	}

#ifdef USE_CCS
	if (isCCSCommand(m_yourCall)) {
		ccsCommandHandler(m_yourCall, m_myCall1, "UR Call"));
		sendToOutgoing(header);
	} else
#endif
	{
		g2CommandHandler(m_yourCall, m_myCall1, header);

		if (m_g2Status == G2_NONE) {
			reflectorCommandHandler(m_yourCall, m_myCall1, "UR Call");
			sendToOutgoing(header);
		}
	}
}

void CRepeaterHandler::processRepeater(CAMBEData& data)
{
	// AMBE data via RF resets the reconnect timer
	m_linkReconnectTimer.start();
	m_watchdogTimer.start();

	m_frames++;
	m_errors += data.getErrors();

	unsigned char buffer[DV_FRAME_MAX_LENGTH_BYTES];
	data.getData(buffer, DV_FRAME_MAX_LENGTH_BYTES);

	// Check for the fast data signature
	if (!m_fastData) {
		unsigned char slowDataType = (buffer[VOICE_FRAME_LENGTH_BYTES] ^ SCRAMBLER_BYTE1) & SLOW_DATA_TYPE_MASK;
		if (slowDataType == SLOW_DATA_TYPE_FAST_DATA1 || slowDataType == SLOW_DATA_TYPE_FAST_DATA2)
			m_fastData = true;
	}

	// Don't do AMBE processing when in Fast Data mode
	if (!m_fastData) {
		if (::memcmp(buffer, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES) == 0)
			m_silence++;

		// Don't do DTMF decoding or blanking if off and not on crossband either
		if (m_dtmfEnabled && m_g2Status != G2_XBAND) {
			bool pressed = m_dtmf.decode(buffer, data.isEnd());
			if (pressed) {
				// Replace the DTMF with silence
				::memcpy(buffer, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);
				data.setData(buffer, DV_FRAME_LENGTH_BYTES);
			}

			bool dtmfDone = m_dtmf.hasCommand();
			if (dtmfDone) {
				std::string command = m_dtmf.translate();

				// Only process the DTMF command if the your call is CQCQCQ and not a restricted user
				if (!m_restricted && m_yourCall.substr(0, 4U) == "CQCQ") {
					if (command.empty()) {
						// Do nothing
					}
#ifdef USE_CCS
					else if (isCCSCommand(command)) {
						ccsCommandHandler(command, m_myCall1, "DTMF"));
					}
#endif
					else if (command == "       I") {
						m_infoNeeded = true;
					} else {
						reflectorCommandHandler(command, m_myCall1, "DTMF");
					}
				}
			}
		}
	}

	// Incoming links get everything
	sendToIncoming(data);

#ifdef USE_CCS
	// CCS gets everything
	m_ccsHandler->writeAMBE(data);
#endif

	if (m_drats != NULL)
		m_drats->writeData(data);

	if (m_outgoingAprsHandler != NULL)
		m_outgoingAprsHandler->writeData(m_rptCallsign, data);

	if (m_text.empty() && !data.isEnd()) {
		m_textCollector.writeData(data);

		bool hasText = m_textCollector.hasData();
		if (hasText) {
			m_text = m_textCollector.getData();
			sendHeard(m_text);
		}
	}

	data.setText(m_text);

	// If no slow data text has been received, send a heard with no text when the end of the
	// transmission arrives
	if (data.isEnd() && m_text.empty())
		sendHeard();

	// Send the statistics after the end of the data, any stats from the repeater should have
	// been received by now
	if (data.isEnd()) {
		m_watchdogTimer.stop();
		sendStats();
	}

	switch (m_g2Status) {
		case G2_LOCAL:
			if (data.isEnd()) {
				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
			}
			break;

		case G2_OK:
			data.setDestination(m_g2Address, G2_DV_PORT);
			m_g2HandlerPool->writeAMBE(data);

			if (data.isEnd()) {
				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
			}
			break;

		case G2_USER:
		case G2_REPEATER:
			// Data ended before the callsign could be resolved
			if (data.isEnd()) {
				m_queryTimer.stop();
				delete m_g2Header;
				m_repeaterId = 0x0U;
				m_g2Status   = G2_NONE;
				m_g2Header   = NULL;
			}
			break;

		case G2_NONE:
			if (data.isEnd())
				m_repeaterId = 0x00U;

			sendToOutgoing(data);
			break;

		case G2_XBAND:
			m_xBandRptr->process(data, DIR_INCOMING, AS_XBAND);

			if (data.isEnd()) {
				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
				m_xBandRptr  = NULL;
			}
			break;

#ifdef USE_STARNET
		case G2_STARNET:
			m_starNet->process(data);

			if (data.isEnd()) {
				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
				m_starNet    = NULL;
			}
			break;
#endif

		case G2_ECHO:
			m_echo->writeData(data);

			if (data.isEnd()) {
				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
			}
			break;

		case G2_VERSION:
			sendToOutgoing(data);

			if (data.isEnd()) {
				m_version->sendVersion();

				m_repeaterId = 0x00U;
				m_g2Status   = G2_NONE;
			}
			break;
	}

	if (data.isEnd() && m_infoNeeded) {
		m_infoAudio->sendStatus();
		m_infoNeeded = false;
	}

#ifdef USE_ANNOUNCE
	if (data.isEnd() && m_msgNeeded) {
		m_msgAudio->sendAnnouncement();
		m_msgNeeded = false;
	}

	if (data.isEnd() && m_wxNeeded) {
		m_wxAudio->sendAnnouncement();
		m_wxNeeded = false;
	}
#endif
}

// Incoming headers when relaying network traffic, as detected by the repeater, will be used as a command
// to the reflector command handler, probably to do an unlink.
void CRepeaterHandler::processBusy(CHeaderData& header)
{
	unsigned int id = header.getId();

	// Ignore duplicate headers
	if (id == m_busyId)
		return;

	std::string rptCall1 = header.getRptCall1();
	std::string rptCall2 = header.getRptCall2();

	if (m_hwType == HW_ICOM) {
		unsigned char band1 = header.getBand1();
		unsigned char band2 = header.getBand2();
		unsigned char band3 = header.getBand3();

		if (m_band1 != band1 || m_band2 != band2 || m_band3 != band3) {
			m_band1 = band1;
			m_band2 = band2;
			m_band3 = band3;
			CLog::logInfo("Repeater %s registered with bands %u %u %u", rptCall1.c_str(), m_band1, m_band2, m_band3);
		}
	}

	if (header.getFlag1() == 0x01) {
		CLog::logInfo("Received a busy message from repeater %s", rptCall1.c_str());
		return;
	}

	// Reject the header if the RPT2 value is not one of the gateway callsigns
	if (rptCall2 != m_gwyCallsign && rptCall2 != m_gateway)
		return;

	m_myCall1  = header.getMyCall1();
	m_yourCall = header.getYourCall();
	m_rptCall1 = rptCall1;
	m_rptCall2 = rptCall2;

	m_dtmf.reset();

	m_busyId     = id;
	m_repeaterId = 0x00U;
	m_watchdogTimer.start();

	// If restricted then don't send to the command handler
	m_restricted = false;
	if (m_restrictList != NULL) {
		bool res = m_restrictList->isInList(m_myCall1);
		if (res) {
			m_restricted = true;
			return;
		}
	}

	// Reject simple cases
	if (m_yourCall.substr(0,4) == "CQCQ" || m_yourCall == "       E" || m_yourCall == "       I")
		return;

#ifdef USE_CCS
	if (isCCSCommand(m_yourCall))
		ccsCommandHandler(m_yourCall, m_myCall1, "background UR Call");
	else
#endif
	reflectorCommandHandler(m_yourCall, m_myCall1, "background UR Call");
}

void CRepeaterHandler::processBusy(CAMBEData& data)
{
	m_watchdogTimer.start();

	unsigned char buffer[DV_FRAME_MAX_LENGTH_BYTES];
	data.getData(buffer, DV_FRAME_MAX_LENGTH_BYTES);

	// Don't do DTMF decoding if off
	if (m_dtmfEnabled) {
		m_dtmf.decode(buffer, data.isEnd());

		bool dtmfDone = m_dtmf.hasCommand();
		if (dtmfDone) {
			std::string command = m_dtmf.translate();

			// Only process the DTMF command if the your call is CQCQCQ and the user isn't restricted
			if (!m_restricted && m_yourCall.substr(0,4U) == "CQCQ") {
				if (command.empty()) {
					// Do nothing
				}
#ifdef USE_CCS
				else if (isCCSCommand(command)) {
					ccsCommandHandler(command, m_myCall1, "background DTMF");
				}
#endif
				else if (command == "       I") {
					// Do nothing
				} else {
					reflectorCommandHandler(command, m_myCall1, "background DTMF");
				}
			}
		}
	}

	if (data.isEnd()) {
		if (m_infoNeeded) {
			m_infoAudio->sendStatus();
			m_infoNeeded = false;
		}
#ifdef USE_ANNOUNCE
		if (m_msgNeeded) {
			m_msgAudio->sendAnnouncement();
			m_msgNeeded = false;
		}

		if (m_wxNeeded) {
			m_wxAudio->sendAnnouncement();
			m_wxNeeded = false;
		}
#endif

		if (m_g2Status == G2_VERSION)
			m_version->sendVersion();

		m_g2Status = G2_NONE;
		m_busyId = 0x00U;
		m_watchdogTimer.stop();
	}
}

void CRepeaterHandler::processRepeater(CHeardData& heard)
{
	if (m_irc == NULL)
		return;

	// A second heard has come in before the original has been sent or cancelled
	if (m_heardTimer.isRunning() && !m_heardTimer.hasExpired())
		m_irc->sendHeard(m_heardUser, "    ", "        ", m_heardRepeater, "        ", 0x00U, 0x00U, 0x00U);

	m_heardUser     = heard.getUser();
	m_heardRepeater = heard.getRepeater();

	m_heardTimer.start();
}

void CRepeaterHandler::processRepeater(CPollData& data)
{
	if (!m_pollTimer.hasExpired())
		return;

	if (m_irc == NULL)
		return;

	std::string callsign = m_rptCallsign;
	if (m_ddMode)
		callsign += "D";

	std::string text = data.getData1();

	m_irc->kickWatchdog(callsign, text);

	m_pollTimer.start();
}

void CRepeaterHandler::processRepeater(CDDData& data)
{
	if (!m_ddMode)
		return;

	if (m_ddCallsign.empty()) {
		m_ddCallsign = data.getYourCall();
		CLog::logInfo("Added DD callsign %s", m_ddCallsign.c_str());
	}

	CDDHandler::process(data);
}

bool CRepeaterHandler::process(CDDData& data)
{
	unsigned char* address = data.getDestinationAddress();
	if (::memcmp(address, ETHERNET_BROADCAST_ADDRESS, ETHERNET_ADDRESS_LENGTH) == 0)
		data.setRepeaters(m_gwyCallsign, "        ");
	else if (::memcmp(address, TOALL_MULTICAST_ADDRESS, ETHERNET_ADDRESS_LENGTH) == 0)
		data.setRepeaters(m_gwyCallsign, m_rptCallsign);
	else if (::memcmp(address, DX_MULTICAST_ADDRESS, ETHERNET_ADDRESS_LENGTH) == 0)
		data.setRepeaters(m_gwyCallsign, m_rptCallsign);
	else
		data.setRepeaters(m_gwyCallsign, m_rptCallsign);

	data.setDestination(m_address, m_port);
	data.setFlags(0xC0U, 0x00U, 0x00U);
	data.setMyCall1(m_ddCallsign);
	data.setMyCall2("    ");

	m_repeaterHandler->writeDD(data);

	return true;
}

bool CRepeaterHandler::process(CHeaderData& header, DIRECTION, AUDIO_SOURCE source)
{
	// If data is coming from the repeater then don't send
	if (m_repeaterId != 0x00U)
		return false;

	// Rewrite the ID if we're using Icom hardware
	if (m_hwType == HW_ICOM) {
		unsigned int id1 = header.getId();
		unsigned int id2 = id1 + m_index;
		header.setId(id2);
	}

	// Send all original headers to all repeater types, and only send duplicate headers to homebrew repeaters
	if (source != AS_DUP || (source == AS_DUP && m_hwType == HW_HOMEBREW)) {
		header.setBand1(m_band1);
		header.setBand2(m_band2);
		header.setBand3(m_band3);
		header.setDestination(m_address, m_port);
		header.setRepeaters(m_gwyCallsign, m_rptCallsign);

		m_repeaterHandler->writeHeader(header);
	}

	// Don't send duplicate headers to anyone else
	if (source == AS_DUP)
		return true;

	if(m_incomingAprsHandler != nullptr)
		m_incomingAprsHandler->writeHeader(m_rptCallsign, header);

	sendToIncoming(header);

#ifdef USE_CCS
	if (source == AS_DPLUS || source == AS_DEXTRA || source == AS_DCS)
		m_ccsHandler->writeHeader(header);
#endif

	if (source == AS_G2 || source == AS_INFO || source == AS_VERSION || source == AS_XBAND || source == AS_ECHO)
		return true;

	// Reset the slow data text collector, used for DCS text passing
	m_textCollector.reset();
	m_text.clear();

	sendToOutgoing(header);

	return true;
}

bool CRepeaterHandler::process(CAMBEData& data, DIRECTION, AUDIO_SOURCE source)
{
	// If data is coming from the repeater then don't send
	if (m_repeaterId != 0x00U)
		return false;

	// Rewrite the ID if we're using Icom hardware
	if (m_hwType == HW_ICOM) {
		unsigned int id = data.getId();
		id += m_index;
		data.setId(id);
	}

	data.setBand1(m_band1);
	data.setBand2(m_band2);
	data.setBand3(m_band3);
	data.setDestination(m_address, m_port);

	m_repeaterHandler->writeAMBE(data);

	if(m_incomingAprsHandler != nullptr)
		m_incomingAprsHandler->writeData(m_rptCallsign, data);

	sendToIncoming(data);

#ifdef USE_CCS
	if (source == AS_DPLUS || source == AS_DEXTRA || source == AS_DCS)
		m_ccsHandler->writeAMBE(data);
#endif

	if (source == AS_G2 || source == AS_INFO || source == AS_VERSION || source == AS_XBAND || source == AS_ECHO)
		return true;

	// Collect the text from the slow data for DCS
	if (m_text.empty() && !data.isEnd()) {
		m_textCollector.writeData(data);

		bool hasText = m_textCollector.hasData();
		if (hasText)
			m_text = m_textCollector.getData();
	}

	data.setText(m_text);

	sendToOutgoing(data);

	return true;
}

void CRepeaterHandler::resolveUserInt(const std::string& user, const std::string& repeater, const std::string& gateway, const std::string &address)
{
	if (m_g2Status == G2_USER && m_g2User == user) {
		m_queryTimer.stop();

		if (!address.empty()) {
			// No point routing to self
			if (repeater == m_rptCallsign) {
				m_g2Status = G2_LOCAL;
				delete m_g2Header;
				m_g2Header = NULL;
				return;
			}

			// User found, update the settings and send the header to the correct place
			m_g2Address.s_addr = ::inet_addr(address.c_str());

			m_g2Repeater = repeater;
			m_g2Gateway  = gateway;

			m_g2Header->setDestination(m_g2Address, G2_DV_PORT);
			m_g2Header->setRepeaters(m_g2Gateway, m_g2Repeater);
			m_g2HandlerPool->writeHeader(*m_g2Header);

			delete m_g2Header;
			m_g2Status = G2_OK;
			m_g2Header = NULL;
		} else {
			// User not found, remove G2 settings
			m_g2Status = G2_LOCAL;
			m_g2User.clear();
			m_g2Repeater.clear();
			m_g2Gateway.clear();

			delete m_g2Header;
			m_g2Header = NULL;
		}
	}
}

void CRepeaterHandler::resolveRepeaterInt(const std::string& repeater, const std::string& gateway, const std::string &address, DSTAR_PROTOCOL protocol)
{
	if (m_g2Status == G2_REPEATER && m_g2Repeater == repeater) {
		m_queryTimer.stop();

		if (!address.empty()) {
			// Repeater found, update the settings and send the header to the correct place
			m_g2Address.s_addr = ::inet_addr(address.c_str());

			m_g2Repeater = repeater;
			m_g2Gateway  = gateway;

			m_g2Header->setDestination(m_g2Address, G2_DV_PORT);
			m_g2Header->setRepeaters(m_g2Gateway, m_g2Repeater);
			m_g2HandlerPool->writeHeader(*m_g2Header);

			delete m_g2Header;
			m_g2Status = G2_OK;
			m_g2Header = NULL;
		} else {
			// Repeater not found, remove G2 settings
			m_g2Status = G2_LOCAL;
			m_g2User.clear();
			m_g2Repeater.clear();
			m_g2Gateway.clear();

			delete m_g2Header;
			m_g2Header = NULL;
		}
	}

	if (m_linkStatus == LS_PENDING_IRCDDB && m_linkRepeater == repeater) {
		m_queryTimer.stop();

		if (!address.empty()) {
			// Repeater found
			in_addr addr;
			switch (protocol) {
				case DP_DPLUS:
					if (m_dplusEnabled) {
						m_linkGateway = gateway;
						unsigned int localPort = 0U;
						addr.s_addr = ::inet_addr(address.c_str());
						CDPlusHandler::link(this, m_rptCallsign, m_linkRepeater, addr, localPort);
						m_linkStatus = LS_LINKING_DPLUS;
					} else {
						CLog::logInfo("Require D-Plus for linking to %s, but D-Plus is disabled", repeater.c_str());
						m_linkStatus = LS_NONE;
						m_linkRepeater.clear();
						m_linkGateway.clear();
						writeNotLinked();
						triggerInfo();
					}
					break;

				case DP_DCS:
					if (m_dcsEnabled) {
						m_linkGateway = gateway;
						addr.s_addr = ::inet_addr(address.c_str());
						CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, addr);
						m_linkStatus = LS_LINKING_DCS;
					} else {
						CLog::logInfo("Require DCS for linking to %s, but DCS is disabled", repeater.c_str());
						m_linkStatus = LS_NONE;
						m_linkRepeater.clear();
						m_linkGateway.clear();
						writeNotLinked();
						triggerInfo();
					}
					break;

				case DP_LOOPBACK:
					m_linkGateway = gateway;
					addr.s_addr = ::inet_addr(address.c_str());
					CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, addr);
					m_linkStatus = LS_LINKING_LOOPBACK;
					break;

				default:
					if (m_dextraEnabled) {
						m_linkGateway = gateway;
						unsigned int localPort = 0U;
						addr.s_addr = ::inet_addr(address.c_str());
						CDExtraHandler::link(this, m_rptCallsign, m_linkRepeater, addr, localPort);
						m_linkStatus = LS_LINKING_DEXTRA;
					} else {
						CLog::logInfo("Require DExtra for linking to %s, but DExtra is disabled", repeater.c_str());
						m_linkStatus = LS_NONE;
						m_linkRepeater.clear();
						m_linkGateway.clear();
						writeNotLinked();
						triggerInfo();
					}
					break;

			}
		} else {
			// Repeater not found
			m_linkStatus = LS_NONE;
			m_linkRepeater.clear();
			m_linkGateway.clear();

			writeNotLinked();
			triggerInfo();
		}
	}
}

void CRepeaterHandler::clockInt(unsigned int ms)
{
	m_infoAudio->clock(ms);
#ifdef USE_ANNOUNCE
	m_msgAudio->clock(ms);
	m_wxAudio->clock(ms);
#endif
	m_echo->clock(ms);
	m_version->clock(ms);

	if(m_aprsUnit != nullptr)
		m_aprsUnit->clock(ms);

	m_linkReconnectTimer.clock(ms);
	m_watchdogTimer.clock(ms);
	m_queryTimer.clock(ms);
	m_heardTimer.clock(ms);
	m_pollTimer.clock(ms);

	// If the reconnect timer has expired
	if (m_linkReconnectTimer.isRunning() && m_linkReconnectTimer.hasExpired()) {
		if (m_linkStatus != LS_NONE && (m_linkStartup.empty() || m_linkStartup == "        ")) {
			// Unlink if linked to something
			CLog::logInfo("Reconnect timer has expired, unlinking %s from %s", m_rptCallsign.c_str(), m_linkRepeater.c_str());

			CDExtraHandler::unlink(this);
			CDPlusHandler::unlink(this);
			CDCSHandler::unlink(this);

			m_linkStatus = LS_NONE;
			m_linkRepeater.clear();

			// Tell the users
			writeNotLinked();
			triggerInfo();
		} else if ((m_linkStatus == LS_NONE && !m_linkStartup.empty() && m_linkStartup != "        ") ||
				   (m_linkStatus != LS_NONE && m_linkRepeater != m_linkStartup)) {
			// Relink if not linked or linked to the wrong reflector
			CLog::logInfo("Reconnect timer has expired, relinking %s to %s", m_rptCallsign.c_str(), m_linkStartup.c_str());

			// Check for just a change of letter
			if (m_linkStatus != LS_NONE) {
				std::string oldCall = m_linkRepeater.substr(0,LONG_CALLSIGN_LENGTH - 1U);
				std::string newCall = m_linkStartup.substr(0,LONG_CALLSIGN_LENGTH - 1U);

				// Just a change of port?
				if (oldCall == newCall) {
					switch (m_linkStatus) {
						case LS_LINKING_DEXTRA:
						case LS_LINKED_DEXTRA:
							m_linkRelink = true;
							m_linkRepeater = m_linkStartup;
							CDExtraHandler::unlink(this, m_linkRepeater);

							m_linkStatus = LS_LINKING_DEXTRA;
							writeLinkingTo(m_linkRepeater);
							triggerInfo();
							break;

						case LS_LINKING_DCS:
						case LS_LINKED_DCS:
							m_linkRelink = true;
							m_linkRepeater = m_linkStartup;
							CDCSHandler::unlink(this, m_linkRepeater);

							m_linkStatus = LS_LINKING_DCS;
							writeLinkingTo(m_linkRepeater);
							triggerInfo();
							break;

						case LS_LINKING_LOOPBACK:
						case LS_LINKED_LOOPBACK:
							m_linkRelink = true;
							m_linkRepeater = m_linkStartup;
							CDCSHandler::unlink(this, m_linkRepeater);

							m_linkStatus = LS_LINKING_LOOPBACK;
							writeLinkingTo(m_linkRepeater);
							triggerInfo();
							break;

						case LS_LINKING_DPLUS:
							m_linkRepeater = m_linkStartup;
							CDPlusHandler::relink(this, m_linkRepeater);
							writeLinkingTo(m_linkRepeater);
							triggerInfo();
							break;

						case LS_LINKED_DPLUS:
							m_linkRepeater = m_linkStartup;
							CDPlusHandler::relink(this, m_linkRepeater);
							writeLinkedTo(m_linkRepeater);
							triggerInfo();
							break;

						default:
							break;
					}

					return;
				}
			}

			CDExtraHandler::unlink(this);
			CDPlusHandler::unlink(this);
			CDCSHandler::unlink(this);

			linkInt(m_linkStartup);
		}

		m_linkReconnectTimer.start();
	}

	// If the ircDDB query timer has expired
	if (m_queryTimer.isRunning() && m_queryTimer.hasExpired()) {
		m_queryTimer.stop();

		if (m_g2Status == G2_USER || m_g2Status == G2_REPEATER) {
			// User or repeater not found in time, remove G2 settings
			CLog::logInfo("ircDDB did not reply within five seconds");

			m_g2Status = G2_LOCAL;
			m_g2User.clear();
			m_g2Repeater.clear();
			m_g2Gateway.clear();

			delete m_g2Header;
			m_g2Header = NULL;
		} else if (m_linkStatus == LS_PENDING_IRCDDB) {
			// Repeater not found in time
			CLog::logInfo("ircDDB did not reply within five seconds");

			m_linkStatus = LS_NONE;
			m_linkRepeater.clear();
			m_linkGateway.clear();

			writeNotLinked();
			triggerInfo();
		} 
#ifdef USE_CCS
		else if (m_linkStatus == LS_LINKING_CCS) {
			// CCS didn't reply in time
			CLog::logInfo("CCS did not reply within five seconds"));

			m_ccsHandler->stopLink();

			m_linkStatus = LS_NONE;
			m_linkRepeater.clear();

			restoreLinks();
		}
#endif
	}

	// Icom heard timer has expired
	if (m_heardTimer.isRunning() && m_heardTimer.hasExpired() && m_irc != NULL) {
		m_irc->sendHeard(m_heardUser, "    ", "        ", m_heardRepeater, "        ", 0x00U, 0x00U, 0x00U);
		m_heardTimer.stop();
	}

	// If the watchdog timer has expired, clean up
	if (m_watchdogTimer.isRunning() && m_watchdogTimer.hasExpired()) {
		CLog::logInfo("Radio watchdog timer for %s has expired", m_rptCallsign.c_str());
		m_watchdogTimer.stop();

		if (m_repeaterId != 0x00U) {
			if (m_text.empty())
				sendHeard();
				
			if (m_drats != NULL)
				m_drats->writeEnd();


			sendStats();

			switch (m_g2Status) {
				case G2_USER:
				case G2_REPEATER:
					m_queryTimer.stop();
					delete m_g2Header;
					m_g2Header = NULL;
					break;

				case G2_XBAND:
					m_xBandRptr = NULL;
					break;
#ifdef USE_STARNET
				case G2_STARNET:
					m_starNet = NULL;
					break;
#endif
				case G2_ECHO:
					m_echo->end();
					break;

				case G2_VERSION:
					m_version->sendVersion();
					break;

				default:
					break;
			}

			if (m_infoNeeded) {
				m_infoAudio->sendStatus();
				m_infoNeeded = false;
			}

#ifdef USE_ANNOUNCE
			if (m_msgNeeded) {
				m_msgAudio->sendAnnouncement();
				m_msgNeeded = false;
			}

			if (m_wxNeeded) {
				m_wxAudio->sendAnnouncement();
				m_wxNeeded = false;
			}
#endif

			m_repeaterId = 0x00U;
			m_g2Status   = G2_NONE;
		}

		if (m_busyId != 0x00U) {
			if (m_infoNeeded) {
				m_infoAudio->sendStatus();
				m_infoNeeded = false;
			}
#ifdef USE_ANNOUNCE
			if (m_msgNeeded) {
				m_msgAudio->sendAnnouncement();
				m_msgNeeded = false;
			}

			if (m_wxNeeded) {
				m_wxAudio->sendAnnouncement();
				m_wxNeeded = false;
			}
#endif

			if (m_g2Status == G2_VERSION)
				m_version->sendVersion();

			m_g2Status = G2_NONE;
			m_busyId = 0x00U;
		}
	}
}

void CRepeaterHandler::linkUp(DSTAR_PROTOCOL protocol, const std::string& callsign)
{
	if (protocol == DP_DEXTRA && m_linkStatus == LS_LINKING_DEXTRA) {
		CLog::logInfo("DExtra link to %s established", callsign.c_str());
		m_linkStatus = LS_LINKED_DEXTRA;
		writeLinkedTo(callsign);
		triggerInfo();
	}

	if (protocol == DP_DPLUS && m_linkStatus == LS_LINKING_DPLUS) {
		CLog::logInfo("D-Plus link to %s established", callsign.c_str());
		m_linkStatus = LS_LINKED_DPLUS;
		writeLinkedTo(callsign);
		triggerInfo();
	}

	if (protocol == DP_DCS && m_linkStatus == LS_LINKING_DCS) {
		CLog::logInfo("DCS link to %s established", callsign.c_str());
		m_linkStatus = LS_LINKED_DCS;
		writeLinkedTo(callsign);
		triggerInfo();
	}

	if (protocol == DP_DCS && m_linkStatus == LS_LINKING_LOOPBACK) {
		CLog::logInfo("Loopback link to %s established", callsign.c_str());
		m_linkStatus = LS_LINKED_LOOPBACK;
		writeLinkedTo(callsign);
		triggerInfo();
	}
}

bool CRepeaterHandler::linkFailed(DSTAR_PROTOCOL protocol, const std::string& callsign, bool isRecoverable)
{
	// Is relink to another module required?
	if (!isRecoverable && m_linkRelink) {
		m_linkRelink = false;
		CLog::logInfo("Relinking %s from %s to %s", m_rptCallsign.c_str(), callsign.c_str(), m_linkRepeater.c_str());
		linkInt(m_linkRepeater);
		return false;
	}

	// Have we linked to something else in the meantime?
	if (m_linkStatus == LS_NONE || m_linkRepeater != callsign) {
		switch (protocol) {
			case DP_DCS:
				CLog::logInfo("DCS link to %s has failed", callsign.c_str());
				break;
			case DP_DEXTRA:
				CLog::logInfo("DExtra link to %s has failed", callsign.c_str());
				break;
			case DP_DPLUS:
				CLog::logInfo("D-Plus link to %s has failed", callsign.c_str());
				break;
			default:
				break;
		}

		return false;
	}

	if (!isRecoverable) {
		if (protocol == DP_DEXTRA && callsign == m_linkRepeater) {
			CLog::logInfo("DExtra link to %s has failed", m_linkRepeater.c_str());
			m_linkRepeater.clear();
			m_linkStatus = LS_NONE;
			writeNotLinked();
			triggerInfo();
		}

		if (protocol == DP_DPLUS && callsign == m_linkRepeater) {
			CLog::logInfo("D-Plus link to %s has failed", m_linkRepeater.c_str());
			m_linkRepeater.clear();
			m_linkStatus = LS_NONE;
			writeNotLinked();
			triggerInfo();
		}

		if (protocol == DP_DCS && callsign == m_linkRepeater) {
			if (m_linkStatus == LS_LINKED_DCS || m_linkStatus == LS_LINKING_DCS)
				CLog::logInfo("DCS link to %s has failed", m_linkRepeater.c_str());
			else
				CLog::logInfo("Loopback link to %s has failed", m_linkRepeater.c_str());
			m_linkRepeater.clear();
			m_linkStatus = LS_NONE;
			writeNotLinked();
			triggerInfo();
		}

		return false;
	}

	if (protocol == DP_DEXTRA) {
		switch (m_linkStatus) {
			case LS_LINKED_DEXTRA:
				CLog::logInfo("DExtra link to %s has failed, relinking", m_linkRepeater.c_str());
				m_linkStatus = LS_LINKING_DEXTRA;
				writeLinkingTo(m_linkRepeater);
				triggerInfo();
				return true;

			case LS_LINKING_DEXTRA:
				return true;

			default:
				return false;
		}
	}

	if (protocol == DP_DPLUS) {
		switch (m_linkStatus) {
			case LS_LINKED_DPLUS:
				CLog::logInfo("D-Plus link to %s has failed, relinking", m_linkRepeater.c_str());
				m_linkStatus = LS_LINKING_DPLUS;
				writeLinkingTo(m_linkRepeater);
				triggerInfo();
				return true;

			case LS_LINKING_DPLUS:
				return true;

			default:
				return false;
		}
	}

	if (protocol == DP_DCS) {
		switch (m_linkStatus) {
			case LS_LINKED_DCS:
				CLog::logInfo("DCS link to %s has failed, relinking", m_linkRepeater.c_str());
				m_linkStatus = LS_LINKING_DCS;
				writeLinkingTo(m_linkRepeater);
				triggerInfo();
				return true;

			case LS_LINKED_LOOPBACK:
				CLog::logInfo("Loopback link to %s has failed, relinking", m_linkRepeater.c_str());
				m_linkStatus = LS_LINKING_LOOPBACK;
				writeLinkingTo(m_linkRepeater);
				triggerInfo();
				return true;

			case LS_LINKING_DCS:
			case LS_LINKING_LOOPBACK:
				return true;

			default:
				return false;
		}
	}

	return false;
}

void CRepeaterHandler::linkRefused(DSTAR_PROTOCOL protocol, const std::string& callsign)
{
	if (protocol == DP_DEXTRA && callsign == m_linkRepeater) {
		CLog::logInfo("DExtra link to %s was refused", m_linkRepeater.c_str());
		m_linkRepeater.clear();
		m_linkStatus = LS_NONE;
		writeIsBusy(callsign);
		triggerInfo();
	}

	if (protocol == DP_DPLUS && callsign == m_linkRepeater) {
		CLog::logInfo("D-Plus link to %s was refused", m_linkRepeater.c_str());
		m_linkRepeater.clear();
		m_linkStatus = LS_NONE;
		writeIsBusy(callsign);
		triggerInfo();
	}

	if (protocol == DP_DCS && callsign == m_linkRepeater) {
		if (m_linkStatus == LS_LINKED_DCS || m_linkStatus == LS_LINKING_DCS)
			CLog::logInfo("DCS link to %s was refused", m_linkRepeater.c_str());
		else
			CLog::logInfo("Loopback link to %s was refused", m_linkRepeater.c_str());
		m_linkRepeater.clear();
		m_linkStatus = LS_NONE;
		writeIsBusy(callsign);
		triggerInfo();
	}
}

void CRepeaterHandler::link(RECONNECT reconnect, const std::string& reflector)
{
#ifdef USE_CCS
	// CCS removal
	if (m_linkStatus == LS_LINKING_CCS || m_linkStatus == LS_LINKED_CCS) {
		CLog::logInfo("Dropping CCS link to %s", m_linkRepeater.c_str());

		m_ccsHandler->stopLink();

		m_linkStatus = LS_NONE;
		m_linkRepeater.clear();
		m_queryTimer.stop();
	}
#endif

	m_linkStartup   = reflector;
	m_linkReconnect = reconnect;

	m_linkReconnectTimer.stop();

	switch (m_linkReconnect) {
		case RECONNECT_5MINS:
			m_linkReconnectTimer.start(5U * 60U);
			break;
		case RECONNECT_10MINS:
			m_linkReconnectTimer.start(10U * 60U);
			break;
		case RECONNECT_15MINS:
			m_linkReconnectTimer.start(15U * 60U);
			break;
		case RECONNECT_20MINS:
			m_linkReconnectTimer.start(20U * 60U);
			break;
		case RECONNECT_25MINS:
			m_linkReconnectTimer.start(25U * 60U);
			break;
		case RECONNECT_30MINS:
			m_linkReconnectTimer.start(30U * 60U);
			break;
		case RECONNECT_60MINS:
			m_linkReconnectTimer.start(60U * 60U);
			break;
		case RECONNECT_90MINS:
			m_linkReconnectTimer.start(90U * 60U);
			break;
		case RECONNECT_120MINS:
			m_linkReconnectTimer.start(120U * 60U);
			break;
		case RECONNECT_180MINS:
			m_linkReconnectTimer.start(180U * 60U);
			break;
		default:
			break;
	}

	// Nothing to do
	if ((m_linkStatus != LS_NONE && m_linkRepeater == reflector) ||
		(m_linkStatus == LS_NONE && (reflector.empty() || reflector == "        ")))
		return;

	// Handle unlinking
	if (m_linkStatus != LS_NONE && (reflector.empty() || reflector == "        ")) {
		CLog::logInfo("Unlinking %s from %s", m_rptCallsign.c_str(), m_linkRepeater.c_str());

		CDExtraHandler::unlink(this);
		CDPlusHandler::unlink(this);
		CDCSHandler::unlink(this);

		m_linkStatus = LS_NONE;
		m_linkRepeater.clear();

		writeNotLinked();
		triggerInfo();

		return;
	}

	CLog::logInfo("Linking %s to %s", m_rptCallsign.c_str(), reflector.c_str());

	// Check for just a change of letter
	if (m_linkStatus != LS_NONE) {
		std::string oldCall = m_linkRepeater.substr(0,LONG_CALLSIGN_LENGTH - 1U);
		std::string newCall = reflector.substr(0,LONG_CALLSIGN_LENGTH - 1U);

		// Just a change of port?
		if (oldCall == newCall) {
			switch (m_linkStatus) {
				case LS_LINKING_DEXTRA:
				case LS_LINKED_DEXTRA:
					m_linkRelink = true;
					m_linkRepeater = reflector;
					CDExtraHandler::unlink(this, m_linkRepeater);

					m_linkStatus = LS_LINKING_DEXTRA;
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
					break;

				case LS_LINKING_DCS:
				case LS_LINKED_DCS:
					m_linkRelink = true;
					m_linkRepeater = reflector;
					CDCSHandler::unlink(this, m_linkRepeater);

					m_linkStatus = LS_LINKING_DCS;
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
					break;

				case LS_LINKING_LOOPBACK:
				case LS_LINKED_LOOPBACK:
					m_linkRelink = true;
					m_linkRepeater = reflector;
					CDCSHandler::unlink(this, m_linkRepeater);

					m_linkStatus = LS_LINKING_LOOPBACK;
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
					break;

				case LS_LINKING_DPLUS:
					m_linkRepeater = reflector;
					CDPlusHandler::relink(this, m_linkRepeater);
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
					break;

				case LS_LINKED_DPLUS:
					m_linkRepeater = reflector;
					CDPlusHandler::relink(this, m_linkRepeater);
					writeLinkedTo(m_linkRepeater);
					triggerInfo();
					break;

				default:
					break;
			}

			return;
		}
	}

	CDExtraHandler::unlink(this);
	CDPlusHandler::unlink(this);
	CDCSHandler::unlink(this);

	linkInt(m_linkStartup);
}

void CRepeaterHandler::unlink(PROTOCOL protocol, const std::string& reflector)
{
#ifdef USE_CCS
	if (protocol == PROTO_CCS) {
		m_ccsHandler->unlink(reflector);
		return;
	}
#endif

	if (m_linkReconnect == RECONNECT_FIXED && m_linkRepeater == (reflector)) {
		CLog::logInfo("Cannot unlink %s because it is fixed", reflector.c_str());
		return;
	}

	switch (protocol) {
		case PROTO_DPLUS:
			CDPlusHandler::unlink(this, reflector, false);
			break;

		case PROTO_DEXTRA:
			CDExtraHandler::unlink(this, reflector, false);
			break;

		case PROTO_DCS:
			CDCSHandler::unlink(this, reflector, false);
			break;

		default:
			break;
	}
}

void CRepeaterHandler::g2CommandHandler(const std::string& callsign, const std::string& user, CHeaderData& header)
{
	if (m_linkStatus == LS_LINKING_CCS || m_linkStatus == LS_LINKED_CCS)
		return;

	if (!callsign.empty() && callsign[0] == '/') {
		if (m_irc == NULL) {
			CLog::logInfo("%s is trying to G2 route with ircDDB disabled", user.c_str());
			m_g2Status = G2_LOCAL;
			return;
		}

		// This a repeater route
		// Convert "/1234567" to "123456 7"
		std::string repeater = callsign.substr(1, LONG_CALLSIGN_LENGTH - 2U);
		repeater += " ";
		repeater += callsign.substr(callsign.length() - 1, 1);

		if (repeater == m_rptCallsign) {
			CLog::logInfo("%s is trying to G2 route to self, ignoring", user.c_str());
			m_g2Status = G2_LOCAL;
			return;
		}

		if (repeater.substr(0,3U) == "REF" || repeater.substr(0,3U) == "XRF" || repeater.substr(0,3U) == "DCS" || repeater.substr(0,3U) == "XLX") {
			CLog::logInfo("%s is trying to G2 route to reflector %s, ignoring", user.c_str(), repeater.c_str());
			m_g2Status = G2_LOCAL;
			return;
		}

		CLog::logInfo("%s is trying to G2 route to repeater %s", user.c_str(), repeater.c_str());

		m_g2Repeater = repeater;
		m_g2User = "CQCQCQ  ";

		CRepeaterData* data = m_cache->findRepeater(m_g2Repeater);
		m_irc->notifyRepeaterG2NatTraversal(m_g2Repeater);

		if (data == NULL) {
			m_g2Status = G2_REPEATER;
			m_irc->findRepeater(m_g2Repeater);
			m_g2Header = new CHeaderData(header);
			m_queryTimer.start();
		} else {
			m_g2Status = G2_OK;
			m_g2Address = data->getAddress();
			m_g2Gateway = data->getGateway();
			header.setDestination(m_g2Address, G2_DV_PORT);
			header.setRepeaters(m_g2Gateway, m_g2Repeater);
			m_g2HandlerPool->writeHeader(header);
			delete data;
		}
	} else if (string_right(callsign, 1) != "L" && string_right(callsign, 1) != "U") {
		if (m_irc == NULL) {
			CLog::logInfo("%s is trying to G2 route with ircDDB disabled", user.c_str());
			m_g2Status = G2_LOCAL;
			return;
		}

		// This a callsign route
		if (callsign.substr(0,3U) == "REF" || callsign.substr(0,3U) ==  "XRF" || callsign.substr(0,3U) == "DCS" || callsign.substr(0,3U) == "XLX") {
			CLog::logInfo("%s is trying to G2 route to reflector %s, ignoring", user.c_str(), callsign.c_str());
			m_g2Status = G2_LOCAL;
			return;
		}

		CLog::logInfo("%s is trying to G2 route to callsign %s", user.c_str(), callsign.c_str());

		CUserData* data = m_cache->findUser(callsign);

		if (data == NULL) {
			m_g2User   = callsign;
			m_g2Status = G2_USER;
			m_irc->findUser(m_g2User);
			m_g2Header = new CHeaderData(header);
			m_queryTimer.start();
		} else {
			// No point G2 routing to yourself
			if (data->getRepeater() == m_rptCallsign) {
				m_g2Status = G2_LOCAL;
				delete data;
				return;
			}

			m_g2Status   = G2_OK;
			m_g2User     = callsign;
			m_g2Address  = data->getAddress();
			m_g2Repeater = data->getRepeater();
			m_irc->notifyRepeaterG2NatTraversal(m_g2Repeater);
			m_g2Gateway  = data->getGateway();
			header.setDestination(m_g2Address, G2_DV_PORT);
			header.setRepeaters(m_g2Gateway, m_g2Repeater);
			m_g2HandlerPool->writeHeader(header);

			delete data;
		}
	}
}

#ifdef USE_CCS
void CRepeaterHandler::ccsCommandHandler(const std::string& callsign, const std::string& user, const std::string& type)
{
	if (callsign.IsSameAs("CA      "))) {
		m_ccsHandler->stopLink(user, type);
	} else {
		CCS_STATUS status = m_ccsHandler->getStatus();
		if (status == CS_CONNECTED) {
			suspendLinks();
			m_queryTimer.start();
			m_linkStatus   = LS_LINKING_CCS;
			m_linkRepeater = callsign.Mid(1U);
			m_ccsHandler->startLink(m_linkRepeater, user, type);
		}
	}
}
#endif

void CRepeaterHandler::reflectorCommandHandler(const std::string& callsign, const std::string& user, const std::string& type)
{
	if (m_linkStatus == LS_LINKING_CCS || m_linkStatus == LS_LINKED_CCS)
		return;

	if (m_linkReconnect == RECONNECT_FIXED)
		return;

	m_queryTimer.stop();

	std::string letter = string_right(callsign,1);

	if (letter == "U") {
		// Ignore duplicate unlink requests
		if (m_linkStatus == LS_NONE)
			return;

		CLog::logInfo("Unlink command issued via %s by %s", type.c_str(), user.c_str());

		CDExtraHandler::unlink(this);
		CDPlusHandler::unlink(this);
		CDCSHandler::unlink(this);

		m_linkStatus = LS_NONE;
		m_linkRepeater.clear();

		writeNotLinked();
		triggerInfo();
	} else if (letter == "L") {
		std::string reflector;

		// Handle the special case of "       L"
		if (callsign == "       L") {
			if (m_linkStartup.empty())
				return;

			reflector = m_linkStartup;
		} else {
			// Extract the callsign "1234567L" -> "123456 7"
			reflector = callsign.substr(0,LONG_CALLSIGN_LENGTH - 2U);
			reflector += " ";
			reflector += callsign.substr(LONG_CALLSIGN_LENGTH - 2U, 1);
		}

		// Ensure duplicate link requests aren't acted on
		if (m_linkStatus != LS_NONE && reflector == m_linkRepeater)
			return;

		// We can't link to ourself
		if (reflector == m_rptCallsign) {
			CLog::logInfo("%s is trying to link with self via %s, ignoring", user.c_str(), type.c_str());
			triggerInfo();
			return;
		}

		CLog::logInfo("Link command from %s to %s issued via %s by %s", m_rptCallsign.c_str(), reflector.c_str(), type.c_str(), user.c_str());

		// Check for just a change of letter
		if (m_linkStatus != LS_NONE) {
			std::string oldCall = m_linkRepeater.substr(0,LONG_CALLSIGN_LENGTH - 1U);
			std::string newCall = reflector.substr(0,LONG_CALLSIGN_LENGTH - 1U);

			// Just a change of port?
			if (oldCall == newCall) {
				switch (m_linkStatus) {
					case LS_LINKING_DEXTRA:
					case LS_LINKED_DEXTRA:
						m_linkRelink = true;
						m_linkRepeater = reflector;
						CDExtraHandler::unlink(this, m_linkRepeater);

						m_linkStatus = LS_LINKING_DEXTRA;
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
						break;

					case LS_LINKING_DCS:
					case LS_LINKED_DCS:
						m_linkRelink = true;
						m_linkRepeater = reflector;
						CDCSHandler::unlink(this, m_linkRepeater);

						m_linkStatus = LS_LINKING_DCS;
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
						break;

					case LS_LINKING_LOOPBACK:
					case LS_LINKED_LOOPBACK:
						m_linkRelink = true;
						m_linkRepeater = reflector;
						CDCSHandler::unlink(this, m_linkRepeater);

						m_linkStatus = LS_LINKING_LOOPBACK;
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
						break;

					case LS_LINKING_DPLUS:
						m_linkRepeater = reflector;
						CDPlusHandler::relink(this, m_linkRepeater);
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
						break;

					case LS_LINKED_DPLUS:
						m_linkRepeater = reflector;
						CDPlusHandler::relink(this, m_linkRepeater);
						writeLinkedTo(m_linkRepeater);
						triggerInfo();
						break;

					default:
						break;
				}

				return;
			}
		}

		CDExtraHandler::unlink(this);
		CDPlusHandler::unlink(this);
		CDCSHandler::unlink(this);

		linkInt(reflector);
	}
}

void CRepeaterHandler::linkInt(const std::string& callsign)
{
	// Find the repeater to link to
	CRepeaterData* data = m_cache->findRepeater(callsign);

	// Are we trying to link to an unknown DExtra, D-Plus, or DCS reflector?
	if (data == NULL && (callsign.substr(0,3U) == "REF" || callsign.substr(0,3U) == "XRF" || callsign.substr(0,3U) == "DCS" || callsign.substr(0,3U) == "XLX")) {
		CLog::logInfo("%s is unknown, ignoring link request", callsign.c_str());
		triggerInfo();
		return;
	}

	m_linkRepeater = callsign;

	if (data != NULL) {
		m_linkGateway = data->getGateway();

		switch (data->getProtocol()) {
			case DP_DPLUS:
				if (m_dplusEnabled) {
					unsigned int localPort = 0U;
					m_linkStatus = LS_LINKING_DPLUS;
					CDPlusHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress(), localPort);
					if(m_irc != nullptr && localPort > 0U)
						m_irc->notifyRepeaterDPlusNatTraversal(m_linkRepeater, localPort);
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
				} else {
					CLog::logInfo("Require D-Plus for linking to %s, but D-Plus is disabled", callsign.c_str());
					m_linkStatus = LS_NONE;
					writeNotLinked();
					triggerInfo();
				}
				break;

			case DP_DCS:
				if (m_dcsEnabled) {
					m_linkStatus = LS_LINKING_DCS;
					CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress());
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
				} else {
					CLog::logInfo("Require DCS for linking to %s, but DCS is disabled", callsign.c_str());
					m_linkStatus = LS_NONE;
					writeNotLinked();
					triggerInfo();
				}
				break;

			case DP_LOOPBACK:
				m_linkStatus = LS_LINKING_LOOPBACK;
				CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress());
				writeLinkingTo(m_linkRepeater);
				triggerInfo();
				break;

			default:
				if (m_dextraEnabled) {
					unsigned int localPort = 0U;
					m_linkStatus = LS_LINKING_DEXTRA;
					CDExtraHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress(), localPort);
					if(m_irc != nullptr && localPort > 0U)
							m_irc->notifyRepeaterDextraNatTraversal(m_linkRepeater, localPort);
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
				} else {
					CLog::logInfo("Require DExtra for linking to %s, but DExtra is disabled", callsign.c_str());
					m_linkStatus = LS_NONE;
					writeNotLinked();
					triggerInfo();
				}
				break;
		}

		delete data;
	} else {
		if (m_irc != NULL) {
			m_linkStatus = LS_PENDING_IRCDDB;
			m_irc->findRepeater(callsign);
			m_queryTimer.start();
			writeLinkingTo(callsign);
			triggerInfo();
		} else {
			m_linkStatus = LS_NONE;
			writeNotLinked();
			triggerInfo();
		}
	}
}

void CRepeaterHandler::sendToOutgoing(const CHeaderData& header)
{
	CHeaderData temp(header);

	temp.setCQCQCQ();
	temp.setFlags(0x00U, 0x00U, 0x00U);

	// Outgoing DPlus links change the RPT1 and RPT2 values in the DPlus handler
	CDPlusHandler::writeHeader(this, temp, DIR_OUTGOING);

	// Outgoing DExtra links have the currently linked repeater/gateway
	// as the RPT1 and RPT2 values
	temp.setRepeaters(m_linkGateway, m_linkRepeater);
	CDExtraHandler::writeHeader(this, temp, DIR_OUTGOING);

	// Outgoing DCS links have the currently linked repeater and repeater callsign
	// as the RPT1 and RPT2 values
	temp.setRepeaters(m_rptCallsign, m_linkRepeater);
	CDCSHandler::writeHeader(this, temp, DIR_OUTGOING);
}

void CRepeaterHandler::sendToOutgoing(const CAMBEData& data)
{
	CAMBEData temp(data);

	CDExtraHandler::writeAMBE(this, temp, DIR_OUTGOING);

	CDPlusHandler::writeAMBE(this, temp, DIR_OUTGOING);

	CDCSHandler::writeAMBE(this, temp, DIR_OUTGOING);
}

void CRepeaterHandler::sendToIncoming(const CHeaderData& header)
{
	CHeaderData temp(header);

	temp.setCQCQCQ();
	temp.setFlags(0x00U, 0x00U, 0x00U);

	// Incoming DPlus links
	temp.setRepeaters(m_rptCallsign, m_gateway);
	CDPlusHandler::writeHeader(this, temp, DIR_INCOMING);

	// Incoming DExtra links have RPT1 and RPT2 swapped
	temp.setRepeaters(m_gwyCallsign, m_rptCallsign);
	CDExtraHandler::writeHeader(this, temp, DIR_INCOMING);

	// Incoming DCS links have RPT1 and RPT2 swapped
	temp.setRepeaters(m_gwyCallsign, m_rptCallsign);
	CDCSHandler::writeHeader(this, temp, DIR_INCOMING);
}

void CRepeaterHandler::sendToIncoming(const CAMBEData& data)
{
	CAMBEData temp(data);

	CDExtraHandler::writeAMBE(this, temp, DIR_INCOMING);

	CDPlusHandler::writeAMBE(this, temp, DIR_INCOMING);

	CDCSHandler::writeAMBE(this, temp, DIR_INCOMING);
}

void CRepeaterHandler::startupInt()
{
	// Report our existence to ircDDB
	if (m_irc != NULL) {
		std::string callsign = m_rptCallsign;
		if (m_ddMode)
			callsign += "D";

		if (m_frequency > 0.0)
			m_irc->rptrQRG(callsign, m_frequency, m_offset, m_range * 1000.0, m_agl);

		if (m_latitude != 0.0 && m_longitude != 0.0)
			m_irc->rptrQTH(callsign, m_latitude, m_longitude, m_description1, m_description2, m_url);
	}

	if(m_outgoingAprsHandler != nullptr) {
		m_outgoingAprsHandler->addReadAPRSCallback(this);
	}

#ifdef USE_CCS
	m_ccsHandler = new CCCSHandler(this, m_rptCallsign, m_index + 1U, m_latitude, m_longitude, m_frequency, m_offset, m_description1, m_description2, m_url, CCS_PORT + m_index);
#endif

#ifdef USE_CCS
	// Start up our CCS link if we are DV mode
	if (!m_ddMode)
		m_ccsHandler->connect();
#endif

	// Link to a startup reflector/repeater
	if (m_linkAtStartup && !m_linkStartup.empty()) {
		CLog::logInfo("Linking %s at startup to %s", m_rptCallsign.c_str(), m_linkStartup.c_str());

		// Find the repeater to link to
		CRepeaterData* data = m_cache->findRepeater(m_linkStartup);

		m_linkRepeater = m_linkStartup;

		if (data != NULL) {
			m_linkGateway = data->getGateway();

			DSTAR_PROTOCOL protocol = data->getProtocol();
			switch (protocol) {
				case DP_DPLUS:
					if (m_dplusEnabled) {
						unsigned int localPort = 0U;
						m_linkStatus = LS_LINKING_DPLUS;
						CDPlusHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress(), localPort);
						if(m_irc != nullptr && localPort > 0U)
							m_irc->notifyRepeaterDPlusNatTraversal(m_linkRepeater, localPort);
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
					} else {
						CLog::logInfo("Require D-Plus for linking to %s, but D-Plus is disabled", m_linkRepeater.c_str());
						m_linkStatus = LS_NONE;
						writeNotLinked();
						triggerInfo();
					}
					break;

				case DP_DCS:
					if (m_dcsEnabled) {
						m_linkStatus = LS_LINKING_DCS;
						CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress());
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
					} else {
						CLog::logInfo("Require DCS for linking to %s, but DCS is disabled", m_linkRepeater.c_str());
						m_linkStatus = LS_NONE;
						writeNotLinked();
						triggerInfo();
					}
					break;

				case DP_LOOPBACK:
					m_linkStatus = LS_LINKING_LOOPBACK;
					CDCSHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress());
					writeLinkingTo(m_linkRepeater);
					triggerInfo();
					break;

				default:
					if (m_dextraEnabled) {
						unsigned int localPort = 0U;
						m_linkStatus = LS_LINKING_DEXTRA;
						CDExtraHandler::link(this, m_rptCallsign, m_linkRepeater, data->getAddress(), localPort);
						if(m_irc != nullptr && localPort > 0U)
							m_irc->notifyRepeaterDextraNatTraversal(m_linkRepeater, localPort);
						writeLinkingTo(m_linkRepeater);
						triggerInfo();
					} else {
						CLog::logInfo("Require DExtra for linking to %s, but DExtra is disabled", m_linkRepeater.c_str());
						m_linkStatus = LS_NONE;
						writeNotLinked();
						triggerInfo();
					}
					break;
			}

			delete data;
		} else {
			if (m_irc != NULL) {
				m_linkStatus = LS_PENDING_IRCDDB;
				m_irc->findRepeater(m_linkStartup);
				m_queryTimer.start();
				writeLinkingTo(m_linkStartup);
				triggerInfo();
			} else {
				m_linkStatus = LS_NONE;
				writeNotLinked();
				triggerInfo();
			}
		}
	} else {
		writeNotLinked();
		triggerInfo();
	}
}

void CRepeaterHandler::writeLinkingTo(const std::string &callsign)
{
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = CStringUtils::string_format("Verbinde mit %s", callsign.c_str());
			break;
		case TL_DANSK:
			text = CStringUtils::string_format("Linker til %s", callsign.c_str());
			break;
		case TL_FRANCAIS:
			text = CStringUtils::string_format("Connexion a %s", callsign.c_str());
			break;
		case TL_ITALIANO:
			text = CStringUtils::string_format("In conn con %s", callsign.c_str());
			break;
		case TL_POLSKI:
			text = CStringUtils::string_format("Linkuje do %s", callsign.c_str());
			break;
		case TL_ESPANOL:
			text = CStringUtils::string_format("Enlazando %s", callsign.c_str());
			break;
		case TL_SVENSKA:
			text = CStringUtils::string_format("Lankar till %s", callsign.c_str());
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = CStringUtils::string_format("Linken naar %s", callsign.c_str());
			break;
		case TL_NORSK:
			text = CStringUtils::string_format("Kobler til %s", callsign.c_str());
			break;
		case TL_PORTUGUES:
			text = CStringUtils::string_format("Conectando, %s", callsign.c_str());
			break;
		default:
			text = CStringUtils::string_format("Linking to %s", callsign.c_str());
			break;
	}

	CTextData textData(m_linkStatus, callsign, text, m_address, m_port);
	m_repeaterHandler->writeText(textData);

	m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
	triggerInfo();

	if(m_outgoingAprsHandler != nullptr)
		m_outgoingAprsHandler->writeStatus(m_rptCallsign, text);

#ifdef USE_CCS
	m_ccsHandler->setReflector();
#endif
}

void CRepeaterHandler::writeLinkedTo(const std::string &callsign)
{
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = CStringUtils::string_format("Verlinkt zu %s", callsign.c_str());
			break;
		case TL_DANSK:
			text = CStringUtils::string_format("Linket til %s", callsign.c_str());
			break;
		case TL_FRANCAIS:
			text = CStringUtils::string_format("Connecte a %s", callsign.c_str());
			break;
		case TL_ITALIANO:
			text = CStringUtils::string_format("Connesso a %s", callsign.c_str());
			break;
		case TL_POLSKI:
			text = CStringUtils::string_format("Polaczony z %s", callsign.c_str());
			break;
		case TL_ESPANOL:
			text = CStringUtils::string_format("Enlazado %s", callsign.c_str());
			break;
		case TL_SVENSKA:
			text = CStringUtils::string_format("Lankad till %s", callsign.c_str());
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = CStringUtils::string_format("Gelinkt met %s", callsign.c_str());
			break;
		case TL_NORSK:
			text = CStringUtils::string_format("Tilkoblet %s", callsign.c_str());
			break;
		case TL_PORTUGUES:
			text = CStringUtils::string_format("Conectado a %s", callsign.c_str());
			break;
		default:
			text = CStringUtils::string_format("Linked to %s", callsign.c_str());
			break;
	}

	CTextData textData(m_linkStatus, callsign, text, m_address, m_port);
	m_repeaterHandler->writeText(textData);

	m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
	triggerInfo();

	if(m_outgoingAprsHandler != nullptr)
		m_outgoingAprsHandler->writeStatus(m_rptCallsign, text);

#ifdef USE_CCS
	m_ccsHandler->setReflector(callsign);
#endif
}

void CRepeaterHandler::writeNotLinked()
{
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = "Nicht verbunden";
			break;
		case TL_DANSK:
			text = "Ikke forbundet";
			break;
		case TL_FRANCAIS:
			text = "Non connecte";
			break;
		case TL_ITALIANO:
			text = "Non connesso";
			break;
		case TL_POLSKI:
			text = "Nie polaczony";
			break;
		case TL_ESPANOL:
			text = "No enlazado";
			break;
		case TL_SVENSKA:
			text = "Ej lankad";
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = "Niet gelinkt";
			break;
		case TL_NORSK:
			text = "Ikke linket";
			break;
		case TL_PORTUGUES:
			text = "Desconectado";
			break;
		default:
			text = "Not linked";
			break;
	}

	CTextData textData(LS_NONE, "", text, m_address, m_port);
	m_repeaterHandler->writeText(textData);

	m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
	triggerInfo();

	if(m_outgoingAprsHandler != nullptr)
		m_outgoingAprsHandler->writeStatus(m_rptCallsign, text);

#ifdef USE_CCS
	m_ccsHandler->setReflector();
#endif
}

void CRepeaterHandler::writeIsBusy(const std::string& callsign)
{
	std::string tempText;
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = "Nicht verbunden";
			tempText = CStringUtils::string_format("%s ist belegt", callsign.c_str());
			break;
		case TL_DANSK:
			text = "Ikke forbundet";
			tempText = CStringUtils::string_format("Optaget fra %s", callsign.c_str());
			break;
		case TL_FRANCAIS:
			text = "Non connecte";
			tempText = CStringUtils::string_format("Occupe par %s", callsign.c_str());
			break;
		case TL_ITALIANO:
			text = "Non connesso";
			tempText = CStringUtils::string_format("Occupado da%s", callsign.c_str());
			break;
		case TL_POLSKI:
			text = "Nie polaczony";
			tempText = CStringUtils::string_format("%s jest zajety", callsign.c_str());
			break;
		case TL_ESPANOL:
			text = "No enlazado";
			tempText = CStringUtils::string_format("%s ocupado", callsign.c_str());
			break;
		case TL_SVENSKA:
			text = "Ej lankad";
			tempText = CStringUtils::string_format("%s ar upptagen", callsign.c_str());
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = "Niet gelinkt";
			tempText = CStringUtils::string_format("%s is bezet", callsign.c_str());
			break;
		case TL_NORSK:
			text = "Ikke linket";
			tempText = CStringUtils::string_format("%s er opptatt", callsign.c_str());
			break;
		case TL_PORTUGUES:
			text = "Desconectado";
			tempText = CStringUtils::string_format("%s, ocupado", callsign.c_str());
			break;
		default:
			text = "Not linked";
			tempText = CStringUtils::string_format("%s is busy", callsign.c_str());
			break;
	}

	CTextData textData1(m_linkStatus, m_linkRepeater, tempText, m_address, m_port, true);
	m_repeaterHandler->writeText(textData1);

	CTextData textData2(m_linkStatus, m_linkRepeater, text, m_address, m_port);
	m_repeaterHandler->writeText(textData2);

	m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
	m_infoAudio->setTempStatus(m_linkStatus, m_linkRepeater, tempText);
	triggerInfo();

	if(m_outgoingAprsHandler != nullptr)
		m_outgoingAprsHandler->writeStatus(m_rptCallsign, text);

#ifdef USE_CCS
	m_ccsHandler->setReflector();
#endif
}

void CRepeaterHandler::ccsLinkMade(const std::string& callsign, DIRECTION direction)
{
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = CStringUtils::string_format("Verlinkt zu %s", callsign.c_str());
			break;
		case TL_DANSK:
			text = CStringUtils::string_format("Linket til %s", callsign.c_str());
			break;
		case TL_FRANCAIS:
			text = CStringUtils::string_format("Connecte a %s", callsign.c_str());
			break;
		case TL_ITALIANO:
			text = CStringUtils::string_format("Connesso a %s", callsign.c_str());
			break;
		case TL_POLSKI:
			text = CStringUtils::string_format("Polaczony z %s", callsign.c_str());
			break;
		case TL_ESPANOL:
			text = CStringUtils::string_format("Enlazado %s", callsign.c_str());
			break;
		case TL_SVENSKA:
			text = CStringUtils::string_format("Lankad till %s", callsign.c_str());
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = CStringUtils::string_format("Gelinkt met %s", callsign.c_str());
			break;
		case TL_NORSK:
			text = CStringUtils::string_format("Tilkoblet %s", callsign.c_str());
			break;
		case TL_PORTUGUES:
			text = CStringUtils::string_format("Conectado a %s", callsign.c_str());
			break;
		default:
			text = CStringUtils::string_format("Linked to %s", callsign.c_str());
			break;
	}

	if (direction == DIR_OUTGOING) {
		suspendLinks();

		m_linkStatus   = LS_LINKED_CCS;
		m_linkRepeater = callsign;
		m_queryTimer.stop();

		CTextData textData(m_linkStatus, callsign, text, m_address, m_port);
		m_repeaterHandler->writeText(textData);

		m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
		triggerInfo();
	} else {
		CTextData textData(m_linkStatus, m_linkRepeater, text, m_address, m_port, true);
		m_repeaterHandler->writeText(textData);

		m_infoAudio->setTempStatus(LS_LINKED_CCS, callsign, text);
		triggerInfo();
	}
}

void CRepeaterHandler::ccsLinkEnded(const std::string&, DIRECTION direction)
{
	std::string tempText;
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = "Nicht verbunden";
			tempText = "CCS ist beendet";
			break;
		case TL_DANSK:
			text = "Ikke forbundet";
			tempText = "CCS er afsluttet";
			break;
		case TL_FRANCAIS:
			text = "Non connecte";
			tempText = "CCS a pris fin";
			break;
		case TL_ITALIANO:
			text = "Non connesso";
			tempText = "CCS e finita";
			break;
		case TL_POLSKI:
			text = "Nie polaczony";
			tempText = "CCS zakonczyl";
			break;
		case TL_ESPANOL:
			text = "No enlazado";
			tempText = "CCS ha terminado";
			break;
		case TL_SVENSKA:
			text = "Ej lankad";
			tempText = "CCS har upphort";
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = "Niet gelinkt";
			tempText = "CCS is afgelopen";
			break;
		case TL_NORSK:
			text = "Ikke linket";
			tempText = "CCS er avsluttet";
			break;
		case TL_PORTUGUES:
			text = "Desconectado";
			tempText = "CCS terminou";
			break;
		default:
			text = "Not linked";
			tempText = "CCS has ended";
			break;
	}

	if (direction == DIR_OUTGOING) {
		m_linkStatus = LS_NONE;
		m_linkRepeater.clear();
		m_queryTimer.stop();

		bool res = restoreLinks();
		if (!res) {
			CTextData textData1(m_linkStatus, m_linkRepeater, tempText, m_address, m_port, true);
			m_repeaterHandler->writeText(textData1);

			CTextData textData2(m_linkStatus, m_linkRepeater, text, m_address, m_port);
			m_repeaterHandler->writeText(textData2);

			m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
			m_infoAudio->setTempStatus(m_linkStatus, m_linkRepeater, tempText);
			triggerInfo();
		}
	} else {
		CTextData textData(m_linkStatus, m_linkRepeater, tempText, m_address, m_port, true);
		m_repeaterHandler->writeText(textData);

		m_infoAudio->setTempStatus(m_linkStatus, m_linkRepeater, tempText);
		triggerInfo();
	}
}

void CRepeaterHandler::ccsLinkFailed(const std::string& dtmf, DIRECTION direction)
{
	std::string tempText;
	std::string text;

	switch (m_language) {
		case TL_DEUTSCH:
			text = "Nicht verbunden";
			tempText = CStringUtils::string_format("%s unbekannt", dtmf.c_str());
			break;
		case TL_DANSK:
			text = "Ikke forbundet";
			tempText = CStringUtils::string_format("%s unknown", dtmf.c_str());
			break;
		case TL_FRANCAIS:
			text = "Non connecte";
			tempText = CStringUtils::string_format("%s inconnu", dtmf.c_str());
			break;
		case TL_ITALIANO:
			text = "Non connesso";
			tempText = CStringUtils::string_format("Sconosciuto %s", dtmf.c_str());
			break;
		case TL_POLSKI:
			text = "Nie polaczony";
			tempText = CStringUtils::string_format("%s nieznany", dtmf.c_str());
			break;
		case TL_ESPANOL:
			text = "No enlazado";
			tempText = CStringUtils::string_format("Desconocido %s", dtmf.c_str());
			break;
		case TL_SVENSKA:
			text = "Ej lankad";
			tempText = CStringUtils::string_format("%s okand", dtmf.c_str());
			break;
		case TL_NEDERLANDS_NL:
		case TL_NEDERLANDS_BE:
			text = "Niet gelinkt";
			tempText = CStringUtils::string_format("%s bekend", dtmf.c_str());
			break;
		case TL_NORSK:
			text = "Ikke linket";
			tempText = CStringUtils::string_format("%s ukjent", dtmf.c_str());
			break;
		case TL_PORTUGUES:
			text = "Desconectado";
			tempText = CStringUtils::string_format("%s desconhecido", dtmf.c_str());
			break;
		default:
			text = "Not linked";
			tempText = CStringUtils::string_format("%s unknown", dtmf.c_str());
			break;
	}

	if (direction == DIR_OUTGOING) {
		m_linkStatus = LS_NONE;
		m_linkRepeater.clear();
		m_queryTimer.stop();

		bool res = restoreLinks();
		if (!res) {
			CTextData textData1(m_linkStatus, m_linkRepeater, tempText, m_address, m_port, true);
			m_repeaterHandler->writeText(textData1);

			CTextData textData2(m_linkStatus, m_linkRepeater, text, m_address, m_port);
			m_repeaterHandler->writeText(textData2);

			m_infoAudio->setStatus(m_linkStatus, m_linkRepeater, text);
			m_infoAudio->setTempStatus(m_linkStatus, m_linkRepeater, tempText);
			triggerInfo();
		}
	} else {
		CTextData textData(m_linkStatus, m_linkRepeater, tempText, m_address, m_port, true);
		m_repeaterHandler->writeText(textData);

		m_infoAudio->setTempStatus(m_linkStatus, m_linkRepeater, tempText);
		triggerInfo();
	}
}

void CRepeaterHandler::writeStatus(CStatusData& statusData)
{
	for (unsigned int i = 0U; i < m_maxRepeaters; i++) {
		if (m_repeaters[i] != NULL) {
			statusData.setDestination(m_repeaters[i]->m_address, m_repeaters[i]->m_port);
			m_repeaters[i]->m_repeaterHandler->writeStatus(statusData);
		}
	}
}

void CRepeaterHandler::sendHeard(const std::string& text)
{
	if (m_irc == NULL)
		return;

	std::string destination;

	if (m_g2Status == G2_OK) {
		destination = m_g2Repeater;
	} else if (m_g2Status == G2_NONE && (m_linkStatus == LS_LINKED_DPLUS || m_linkStatus == LS_LINKED_DEXTRA || m_linkStatus == LS_LINKED_DCS)) {
		if (m_linkRepeater.substr(0,3U) == "REF" || m_linkRepeater.substr(0,3U) == "XRF" || m_linkRepeater.substr(0,3U) == "DCS" || m_linkRepeater.substr(0,3U) == "XLX")
			destination = m_linkRepeater;
	}

	m_irc->sendHeardWithTXMsg(m_myCall1, m_myCall2, m_yourCall, m_rptCall1, m_rptCall2, m_flag1, m_flag2, m_flag3, destination, text);
}

void CRepeaterHandler::sendStats()
{
	if (m_irc != NULL)
		m_irc->sendHeardWithTXStats(m_myCall1, m_myCall2, m_yourCall, m_rptCall1, m_rptCall2, m_flag1, m_flag2, m_flag3, m_frames, m_silence, m_errors / 2U);
}

void CRepeaterHandler::suspendLinks()
{
	if (m_linkStatus == LS_LINKING_DCS      || m_linkStatus == LS_LINKED_DCS    ||
        m_linkStatus == LS_LINKING_DEXTRA   || m_linkStatus == LS_LINKED_DEXTRA ||
	    m_linkStatus == LS_LINKING_DPLUS    || m_linkStatus == LS_LINKED_DPLUS  ||
		m_linkStatus == LS_LINKING_LOOPBACK || m_linkStatus == LS_LINKED_LOOPBACK) {
		m_lastReflector = m_linkRepeater;
		boost::trim(m_lastReflector);
	}

	CDPlusHandler::unlink(this);
	CDExtraHandler::unlink(this);
	CDCSHandler::unlink(this);

	m_linkStatus = LS_NONE;
	m_linkRepeater.clear();
	m_linkReconnectTimer.stop();

#ifdef USE_CCS
	m_ccsHandler->setReflector();
#endif
}

bool CRepeaterHandler::restoreLinks()
{
	if (m_linkReconnect == RECONNECT_FIXED) {
		if (!m_lastReflector.empty()) {
			linkInt(m_linkStartup);
			m_lastReflector.clear();
			return true;
		}
	} else if (m_linkReconnect == RECONNECT_NEVER) {
		if (!m_lastReflector.empty()) {
			linkInt(m_lastReflector);
			m_lastReflector.clear();
			return true;
		}
	} else {
		m_linkReconnectTimer.start();
		if (!m_lastReflector.empty()) {
			linkInt(m_lastReflector);
			m_lastReflector.clear();
			return true;
		}
	}

	m_lastReflector.clear();
	return false;
}

void CRepeaterHandler::triggerInfo()
{
	if (!m_infoEnabled)
		return;

	// Either send the audio now, or queue it until the end of the transmission
	if (m_repeaterId != 0x00U || m_busyId != 0x00U) {
		m_infoNeeded = true;
	} else {
		m_infoAudio->sendStatus();
		m_infoNeeded = false;
	}
}

void CRepeaterHandler::readAPRSFrame(CAPRSFrame& frame)
{
	if(m_aprsUnit != nullptr) {
		m_aprsUnit->writeFrame(frame);
	}
}

#ifdef USE_CCS
bool CRepeaterHandler::isCCSCommand(const std::string& command) const
{
	if (command.IsSameAs("CA      ")))
		return true;

	wxChar c = command.at(0U);
	if (c != wxT('C'))
		return false;

	c = command.at(1U);
	if (c < wxT('0') || c > wxT('9'))
		return false;

	c = command.at(2U);
	if (c < wxT('0') || c > wxT('9'))
		return false;

	c = command.at(3U);
	if (c < wxT('0') || c > wxT('9'))
		return false;

	return true;
}
#endif
