/*
 *   Copyright (C) 2010,2011,2012 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
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

#include <string>
#include <sstream>
#include <iostream>

#include "Utils.h"
#include "DStarGatewayConfig.h"
#include "DStarDefines.h"
#include "Log.h"

CDStarGatewayConfig::CDStarGatewayConfig(const std::string &pathname)
: m_fileName(pathname)
{

}

bool CDStarGatewayConfig::load()
{
	bool ret = false;
	CLog::logInfo("Loading Configuration from %s", m_fileName.c_str());
	CConfig cfg(m_fileName);

	ret = open(cfg);
	if(ret) {
		ret = loadGateway(cfg) && ret;
		ret = loadIrcDDB(cfg) && ret;
		ret = loadRepeaters(cfg) && ret;
		ret = loadPaths(cfg) && ret;
		ret = loadLog(cfg) && ret;
		ret = loadAPRS(cfg) && ret;
		ret = loadDextra(cfg) && ret;
		ret = loadDCS(cfg) && ret;
		ret = loadDPlus(cfg) && ret;
		ret = loadRemote(cfg) && ret;
		ret = loadXLX(cfg) && ret;
#ifdef USE_GPSD
		ret = loadGPSD(cfg) && ret;
#endif
		ret = loadDaemon(cfg) && ret;
		ret = loadAccessControl(cfg) && ret;
		ret = loadDRats(cfg) && ret;
	}

	if(ret) {
		//properly size values
		m_gateway.callsign.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
		m_gateway.callsign.push_back('G');
	}
	else {
		CLog::logError("Loading Configuration from %s failed", m_fileName.c_str());
	}

	return ret;
}

bool CDStarGatewayConfig::loadDaemon(const CConfig & cfg)
{
	bool ret = cfg.getValue("daemon", "daemon", m_daemon.daemon, false);
	ret = cfg.getValue("daemon", "pidfile", m_daemon.pidFile, 0, 1024, "") && ret;
	ret = cfg.getValue("daemon", "user", m_daemon.user, 0, 1024, "") && ret;
	return ret;
}

bool CDStarGatewayConfig::loadXLX(const CConfig & cfg)
{
	bool ret = cfg.getValue("xlx", "enabled", m_xlx.enabled, true);
	ret = cfg.getValue("xlx", "hostfileUrl", m_xlx.url, 0, 1024, "") && ret;

	m_xlx.enabled = m_xlx.enabled && !m_xlx.url.empty();

	return ret;
}

bool CDStarGatewayConfig::loadRemote(const CConfig & cfg)
{
	bool ret = cfg.getValue("remote", "enabled", m_remote.enabled, false);
	ret = cfg.getValue("remote", "port", m_remote.port, 1U, 65535U, 4242U) && ret;
	ret = cfg.getValue("remote", "password", m_remote.password, 0, 1024, "") && ret;

	m_remote.enabled = m_remote.enabled && !m_remote.password.empty();

	return ret;
}

bool CDStarGatewayConfig::loadDextra(const CConfig & cfg)
{
	bool ret = cfg.getValue("dextra", "enabled", m_dextra.enabled, true);
	ret = cfg.getValue("dextra", "maxDongles", m_dextra.maxDongles, 1U, 5U, 5U) && ret;
	return ret;
}

bool CDStarGatewayConfig::loadDPlus(const CConfig & cfg)
{
	bool ret = cfg.getValue("dplus", "enabled", m_dplus.enabled, true);
	ret = cfg.getValue("dplus", "maxDongles", m_dplus.maxDongles, 1U, 5U, 5U) && ret;
	ret = cfg.getValue("dplus", "login", m_dplus.login, 0, LONG_CALLSIGN_LENGTH, m_gateway.callsign) && ret;

	m_dplus.enabled = m_dplus.enabled && !m_dplus.login.empty();
	m_dplus.login = CUtils::ToUpper(m_dplus.login);

	return ret;
}

bool CDStarGatewayConfig::loadDCS(const CConfig & cfg)
{
	bool ret = cfg.getValue("dcs", "enabled", m_dcs.enabled, true);
	return ret;
}

bool CDStarGatewayConfig::loadAPRS(const CConfig & cfg)
{
	bool ret = cfg.getValue("aprs", "enabled", m_aprs.enabled, false);
	ret = cfg.getValue("aprs", "port", m_aprs.port, 1U, 65535U, 14580U) && ret;
	ret = cfg.getValue("aprs", "hostname", m_aprs.hostname, 0, 1024, "rotate.aprs2.net") && ret;
	ret = cfg.getValue("aprs", "password", m_aprs.password, 0U, 30U, "") && ret;
#ifdef USE_GPSD
	std::string positionSource;
	ret = cfg.getValue("aprs", "positionSource", positionSource, "fixed", {"fixed", "gpsd"}) && ret;
	if(ret) {
		if(positionSource == "fixed")	m_aprs.m_positionSource = POSSRC_FIXED;
		else if(positionSource == "gpsd")	m_aprs.m_positionSource = POSSRC_GPSD;
	}
#else
	m_aprs.m_positionSource = POSSRC_FIXED;
#endif

	m_aprs.enabled = m_aprs.enabled && !m_aprs.password.empty();

	return ret;
}

bool CDStarGatewayConfig::loadLog(const CConfig & cfg)
{
	bool ret = cfg.getValue("log", "path", m_log.logDir, 0, 2048, "/var/log/dstargateway/");
	if(ret && m_log.logDir[m_log.logDir.length() - 1] != '/') {
		m_log.logDir.push_back('/');
	}

	ret = cfg.getValue("log", "fileRoot", m_log.fileRoot, 0, 64, "dstargateway") && ret;
	ret = cfg.getValue("log", "fileRotate", m_log.fileRotate, true) && ret;

	std::string levelStr;
	ret = cfg.getValue("log", "fileLevel", levelStr, "info", {"trace", "debug", "info", "warning", "error", "fatal", "none"}) && ret;
	if(ret) {
		if(levelStr == "trace")			m_log.fileLevel = LOG_TRACE;
		else if(levelStr == "debug")	m_log.fileLevel = LOG_DEBUG;
		else if(levelStr == "info")		m_log.fileLevel = LOG_INFO;
		else if(levelStr == "warning")	m_log.fileLevel = LOG_WARNING;
		else if(levelStr == "error")	m_log.fileLevel = LOG_ERROR;
		else if(levelStr == "fatal")	m_log.fileLevel = LOG_FATAL;
		else if(levelStr == "none")		m_log.fileLevel = LOG_NONE;
	}

	ret = cfg.getValue("log", "displayLevel", levelStr, "info", {"trace", "debug", "info", "warning", "error", "fatal", "none"}) && ret;
	if(ret) {
		if(levelStr == "trace")			m_log.displayLevel = LOG_TRACE;
		else if(levelStr == "debug")	m_log.displayLevel = LOG_DEBUG;
		else if(levelStr == "info")		m_log.displayLevel = LOG_INFO;
		else if(levelStr == "warning")	m_log.displayLevel = LOG_WARNING;
		else if(levelStr == "error")	m_log.displayLevel = LOG_ERROR;
		else if(levelStr == "fatal")	m_log.displayLevel = LOG_FATAL;
		else if(levelStr == "none")		m_log.displayLevel = LOG_NONE;
	}

	return ret;
}

bool CDStarGatewayConfig::loadPaths(const CConfig & cfg)
{
	bool ret = cfg.getValue("paths", "data", m_paths.dataDir, 0, 2048, "/usr/local/share/dstargateway.d/");

	if(ret && m_paths.dataDir[m_paths.dataDir.length() - 1] != '/') {
		m_paths.dataDir.push_back('/');
	}

	//TODO 20211226 check if directory are accessible

	return ret;
}

bool CDStarGatewayConfig::loadRepeaters(const CConfig & cfg)
{
	m_repeaters.clear();
	for(unsigned int i = 0; i < 4; i++) {
		std::string section = CStringUtils::string_format("repeater_%d", i+ 1);
		bool repeaterEnabled;

		bool ret = cfg.getValue(section, "enabled", repeaterEnabled, false);
		if(!ret || !repeaterEnabled)
			continue;
		
		TRepeater * repeater = new TRepeater;
		ret = cfg.getValue(section, "band", repeater->band, 1, 2, "B") && ret;
		ret = cfg.getValue(section, "callsign", repeater->callsign, 0, LONG_CALLSIGN_LENGTH - 1, m_gateway.callsign);
		ret = cfg.getValue(section, "address", repeater->address, 0, 15, "127.0.0.1") && ret;
		ret = cfg.getValue(section, "port", repeater->port, 1U, 65535U, 20011U) && ret;

		std::string hwType;
		ret = cfg.getValue(section, "type", hwType, "", {"hb", "icom"}) && ret;
		if(ret) {
			if(hwType == "hb") 			repeater->hwType = HW_HOMEBREW;
			else if(hwType == "icom")	repeater->hwType = HW_ICOM;
		}

		ret = cfg.getValue(section, "reflector", repeater->reflector, 0, LONG_CALLSIGN_LENGTH, "") && ret;
		ret = cfg.getValue(section, "reflectorAtStartup", repeater->reflectorAtStartup, !repeater->reflector.empty()) && ret;

		std::string reconnect;
		ret = cfg.getValue(section, "reflectorReconnect", reconnect, "never", {"never", "fixed", "5", "10", "15", "20", "25", "30", "60", "90", "120", "180"}) && ret;
		if(ret) {
			if(reconnect == "never")		repeater->reflectorReconnect = RECONNECT_NEVER;
			else if(reconnect == "5")		repeater->reflectorReconnect = RECONNECT_5MINS;
			else if(reconnect == "10")		repeater->reflectorReconnect = RECONNECT_10MINS;
			else if(reconnect == "15")		repeater->reflectorReconnect = RECONNECT_15MINS;
			else if(reconnect == "20")		repeater->reflectorReconnect = RECONNECT_20MINS;
			else if(reconnect == "25")		repeater->reflectorReconnect = RECONNECT_25MINS;
			else if(reconnect == "30")		repeater->reflectorReconnect = RECONNECT_30MINS;
			else if(reconnect == "60")		repeater->reflectorReconnect = RECONNECT_60MINS;
			else if(reconnect == "90")		repeater->reflectorReconnect = RECONNECT_90MINS;
			else if(reconnect == "120")		repeater->reflectorReconnect = RECONNECT_120MINS;
			else if(reconnect == "180")		repeater->reflectorReconnect = RECONNECT_180MINS;
			else if(reconnect == "fixed")	repeater->reflectorReconnect = RECONNECT_FIXED;
		}

		ret = cfg.getValue(section, "frequency", repeater->frequency, 0.1, 1500.0, 434.0) && ret;	
		ret = cfg.getValue(section, "offset", repeater->offset, -50.0, 50.0, 0.0) && ret;
		ret = cfg.getValue(section, "rangeKm", repeater->range, 0.0, 3000.0, 0.0) && ret;
		ret = cfg.getValue(section, "latitude", repeater->latitude, -90.0, 90.0, m_gateway.latitude) && ret;
		ret = cfg.getValue(section, "longitude", repeater->longitude, -180.0, 180.0, m_gateway.longitude) && ret;
		ret = cfg.getValue(section, "agl", repeater->agl, 0, 1000.0, 0.0) && ret;
		ret = cfg.getValue(section, "description1", repeater->description1, 0, 1024, "") && ret;
		ret = cfg.getValue(section, "description2", repeater->description2, 0, 1024, "") && ret;
		ret = cfg.getValue(section, "url", m_gateway.url, 0, 1024, "") && ret;;
		ret = cfg.getValue(section, "band1", repeater->band1, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "band2", repeater->band2, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "band3", repeater->band3, 0, 255, 0) && ret;

		if(ret) {
			m_repeaters.push_back(repeater);
		}
		else {
			delete repeater;
		}
	}

	if(m_repeaters.size() == 0U) {
		CLog::logError("Configuration error: no repeaters configured !");
		return false;
	}

	return true;
}

bool CDStarGatewayConfig::loadIrcDDB(const CConfig & cfg)
{
	bool ret = true;
	for(unsigned int i = 0; i < 4; i++) {
		std::string section = CStringUtils::string_format("ircddb_%d", i + 1);
		bool ircEnabled;

		ret = cfg.getValue(section, "enabled", ircEnabled, false) && ret;
		if(!ircEnabled)
			continue;
		
		TircDDB * ircddb = new TircDDB;
		ret = cfg.getValue(section, "hostname", ircddb->hostname, 0, 1024, "ircv4.openquad.net") && ret;
		ret = cfg.getValue(section, "username", ircddb->username, 0, LONG_CALLSIGN_LENGTH - 1, m_gateway.callsign) && ret;
		ret = cfg.getValue(section, "password", ircddb->password, 0, 1024, "") && ret;

		if(ret) {
			m_ircDDB.push_back(ircddb);
		}
		else {
			delete ircddb;
		}
	}

	return ret;
}

bool CDStarGatewayConfig::loadGateway(const CConfig & cfg)
{
	bool ret = cfg.getValue("gateway", "callsign", m_gateway.callsign, 3, 8, "");
	ret = cfg.getValue("gateway", "address", m_gateway.address, 0, 20, "0.0.0.0") && ret;
	ret = cfg.getValue("gateway", "hbAddress", m_gateway.hbAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("gateway", "hbPort", m_gateway.hbPort, 1U, 65535U, 20010U) && ret;
	ret = cfg.getValue("gateway", "icomAddress", m_gateway.icomAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("gateway", "icomPort", m_gateway.icomPort, 1U, 65535U, 20000U) && ret;
	ret = cfg.getValue("gateway", "latitude", m_gateway.latitude, -90.0, 90.0, 0.0) && ret;
	ret = cfg.getValue("gateway", "longitude", m_gateway.longitude, -180.0, 180.0, 0.0) && ret;
	ret = cfg.getValue("gateway", "description1", m_gateway.description1, 0, 1024, "") && ret;
	ret = cfg.getValue("gateway", "description2", m_gateway.description2, 0, 1024, "") && ret;
	ret = cfg.getValue("gateway", "url", m_gateway.url, 0, 1024, "") && ret;
	
	std::string type;
	ret = cfg.getValue("gateway", "type", type, "repeater", {"repeater", "hotspot"}) && ret;
	if(type == "repeater")		m_gateway.type = GT_REPEATER;
	else if(type == "hotspot")	m_gateway.type = GT_HOTSPOT;

	std::string lang;
	ret = cfg.getValue("gateway", "language", lang, "english_uk",
						{"english_uk", "deutsch", "dansk", "francais", "italiano", "polski",
						"english_us", "espanol", "svenska", "nederlands_nl", "nederlands_be", "norsk", "portugues"}) && ret;;
	if(lang == "english_uk")		m_gateway.language = TL_ENGLISH_UK;
	else if(lang == "deutsch")		m_gateway.language = TL_DEUTSCH;
	else if(lang == "dansk")		m_gateway.language = TL_DANSK;
	else if(lang == "francais")		m_gateway.language = TL_FRANCAIS;
	else if(lang == "italiano") 	m_gateway.language = TL_ITALIANO;
	else if(lang == "polski")		m_gateway.language = TL_POLSKI;
	else if(lang == "english_us")	m_gateway.language = TL_ENGLISH_US;
	else if(lang == "espanol")		m_gateway.language = TL_ESPANOL;
	else if(lang == "svenska")		m_gateway.language = TL_SVENSKA;
	else if(lang == "nederlands_nl")m_gateway.language = TL_NEDERLANDS_NL;
	else if(lang == "nederlands_be")m_gateway.language = TL_NEDERLANDS_BE;
	else if(lang == "norsk")		m_gateway.language = TL_NORSK;
	else if(lang == "portugues")	m_gateway.language = TL_PORTUGUES;

	CUtils::ToUpper(m_gateway.callsign);
	CUtils::clean(m_gateway.description1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.description2, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.url, 		  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");

	return ret;
}

#ifdef USE_GPSD
bool CDStarGatewayConfig::loadGPSD(const CConfig & cfg)
{
	bool ret = cfg.getValue("gpsd", "address", m_gpsd.m_address, 0U, 15U, "127.0.0.1");
	ret = cfg.getValue("gpsd", "port", m_gpsd.m_port, 0U, 5U, "2947") && ret;

	return ret;
}
#endif

bool CDStarGatewayConfig::loadAccessControl(const CConfig & cfg)
{
	bool ret = cfg.getValue("AccessControl", "whiteList", m_accessControl.whiteList, 0U, 2048U, "");
	ret = cfg.getValue("AccessControl", "blackList", m_accessControl.blackList, 0U, 2048U, "") && ret;
	ret = cfg.getValue("AccessControl", "restrictList", m_accessControl.restrictList, 0U, 2048U, "") && ret;
	
	return ret;
}

bool CDStarGatewayConfig::loadDRats(const CConfig & cfg)
{
	bool ret = cfg.getValue("DRats", "enabled", m_drats.enabled, false);

	return ret;
}

bool CDStarGatewayConfig::open(CConfig & cfg)
{
	try {
		return cfg.load();
	}
	catch(...) {
		CLog::logError("Can't read %s\n", m_fileName.c_str());
		return false;
	}
	return true;
}

CDStarGatewayConfig::~CDStarGatewayConfig()
{
	while (m_repeaters.size()) {
		delete m_repeaters.back();
		m_repeaters.pop_back();
	}

	while(m_ircDDB.size()) {
		delete m_ircDDB.back();
		m_ircDDB.pop_back();
	}
}

void CDStarGatewayConfig::getGateway(TGateway & gateway) const
{
	gateway = m_gateway;
}

void CDStarGatewayConfig::getIrcDDB(unsigned int ircddb, TircDDB & ircDDB) const
{
	ircDDB = *(m_ircDDB[ircddb]);
}

unsigned int CDStarGatewayConfig::getRepeaterCount() const
{
	return m_repeaters.size();
}

unsigned int CDStarGatewayConfig::getIrcDDBCount() const
{
	return m_ircDDB.size();
}

void CDStarGatewayConfig::getRepeater(unsigned int index, TRepeater & repeater) const
{
	repeater = *(m_repeaters[index]);
}

void CDStarGatewayConfig::getLog(TLog & log) const
{
	log = m_log;
}

void CDStarGatewayConfig::getPaths(Tpaths & paths) const
{
	paths = m_paths;
}

void CDStarGatewayConfig::getAPRS(TAPRS & aprs) const
{
	aprs = m_aprs;
}

void CDStarGatewayConfig::getDExtra(TDextra & dextra) const
{
	dextra = m_dextra;
}

void CDStarGatewayConfig::getDPlus(TDplus & dplus) const
{
	dplus = m_dplus;
}

void CDStarGatewayConfig::getDCS(TDCS & dcs) const
{
	dcs = m_dcs;
}

void CDStarGatewayConfig::getRemote(TRemote & remote) const
{
	remote = m_remote;
}

void CDStarGatewayConfig::getXLX(TXLX & xlx) const
{
	xlx = m_xlx;
}

#ifdef USE_GPSD
void CDStarGatewayConfig::getGPSD(TGPSD & gpsd) const
{
	gpsd = m_gpsd;
}
#endif

void CDStarGatewayConfig::getDaemon(TDaemon & gen) const
{
	gen = m_daemon;
}

void CDStarGatewayConfig::getAccessControl(TAccessControl & accessControl) const
{
	accessControl = m_accessControl;
}

void CDStarGatewayConfig::getDRats(TDRats & drats) const
{
	drats = m_drats;
}