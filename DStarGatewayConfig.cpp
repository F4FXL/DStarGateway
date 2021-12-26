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

CDStarGatewayConfig::CDStarGatewayConfig(const std::string &pathname)
: m_fileName(pathname)
{

}

bool CDStarGatewayConfig::load()
{
	Config cfg;
	if(open(cfg)
	&& loadGateway(cfg)
	&& loadIrcDDB(cfg)
	&& loadRepeaters(cfg)) {
		return true;
	}

	return false;
}

bool CDStarGatewayConfig::loadPaths(const Config & cfg)
{
	get_value(cfg, "paths.log", m_paths.logDir, 0, 2048, "/var/log/dstargateway/", true);
	get_value(cfg, "paths.data", m_paths.dataDir, 0, 2048, "/var/log/dstargateway/", true);

	if(m_paths.logDir[m_paths.logDir.length() - 1] != '/') {
		m_paths.logDir.push_back('/');
	}

	if(m_paths.dataDir[m_paths.dataDir.length() - 1] != '/') {
		m_paths.dataDir.push_back('/');
	}

	//TODO 20211226 check if directory are accessible

	return true;
}

bool CDStarGatewayConfig::loadRepeaters(const Config & cfg)
{
	cfg.lookup("repeaters");
	// repeater parameters
	for (int i=0; i<cfg.lookup("repeaters").getLength(); i++) {
		std::stringstream key;
		bool isOk = false;
		TRepeater * repeater = new TRepeater;
		key << "repeaters.[" << i << "]";

		if (get_value(cfg, key.str() + ".callsign", repeater->callsign, 0, 7, m_gateway.callsign, true)) {
			CUtils::ToUpper(repeater->callsign);
		}

		if(get_value(cfg, key.str() + ".band", repeater->band, 1, 1, "A")) {
			CUtils::ToUpper(repeater->band);
		}

		if (get_value(cfg, key.str() + ".address", repeater->address, 0, 15, "127.0.0.1", true)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".port", repeater->port, 1U, 65535U, 20011U)) {
			// ???
		}

		std::string hwType;
		if(get_value(cfg, key.str() + ".type", hwType, 1, 5, "", false, {"hb", "icom"} )) {
			if(hwType == "hb") repeater->hwType = HW_HOMEBREW;
			else if(hwType == "icom") repeater->hwType = HW_ICOM;
		}

		if (get_value(cfg, key.str() + ".reflector", repeater->reflector, 0, LONG_CALLSIGN_LENGTH, "", true)) {
			// ???
		}

		if (get_value(cfg, key.str() + ".reflectorAtStartup", repeater->reflectorAtStartup, true)) {
			// ???
		}

		std::string reconnect;
		if (get_value(cfg, key.str() + ".reflectorReconnect", reconnect, 1, 5, "never", true,
						{"never", "fixed", "5", "10", "15", "20", "25", "30", "60", "90", "120", "180"})) {
			if(reconnect == "never") repeater->reflectorReconnect = RECONNECT_NEVER;
			else if(reconnect == "5") repeater->reflectorReconnect = RECONNECT_5MINS;
			else if(reconnect == "10") repeater->reflectorReconnect = RECONNECT_10MINS;
			else if(reconnect == "15") repeater->reflectorReconnect = RECONNECT_15MINS;
			else if(reconnect == "20") repeater->reflectorReconnect = RECONNECT_20MINS;
			else if(reconnect == "25") repeater->reflectorReconnect = RECONNECT_25MINS;
			else if(reconnect == "30") repeater->reflectorReconnect = RECONNECT_30MINS;
			else if(reconnect == "60") repeater->reflectorReconnect = RECONNECT_60MINS;
			else if(reconnect == "90") repeater->reflectorReconnect = RECONNECT_90MINS;
			else if(reconnect == "120") repeater->reflectorReconnect = RECONNECT_120MINS;
			else if(reconnect == "180") repeater->reflectorReconnect = RECONNECT_180MINS;
			else if(reconnect == "fixed") repeater->reflectorReconnect = RECONNECT_FIXED;
		}
		
		if(get_value(cfg, key.str() + ".frequency", repeater->frequency, 0.0, 1500.0, 434.0)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".offset", repeater->offset, 0.0, 50.0, 0.0)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".rangeKm", repeater->range, 0.0, 3000.0, 0.0)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".latitude", repeater->latitude, -90, 90.0, m_gateway.latitude)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".longitude", repeater->longitude, -180, 180.0, m_gateway.longitude)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".agl", repeater->agl, 0, 1000.0, 0.0)) {
			// ???
		}

		if(get_value(cfg, key.str() + ".description1", m_gateway.description1, 0, 1024, "")) {
			// ???
		}

		if(get_value(cfg, key.str() + ".description2", m_gateway.description2, 0, 1024, "")) {
			// ???
		}

		if(get_value(cfg, key.str() + ".url", m_gateway.url, 0, 1024, "")) {
			// ???
		}

		int band;
		if(get_value(cfg, key.str() + ".band1", band, 0, 255, 0)) {
			repeater->band1 = (unsigned char)band;
		}
		if(get_value(cfg, key.str() + ".band2", band, 0, 255, 0)) {
			repeater->band2 = (unsigned char)band;
		}
		if(get_value(cfg, key.str() + ".band3", band, 0, 255, 0)) {
			repeater->band3 = (unsigned char)band;
		}

		// We have read a complete repeater record, validate it
		if(repeater->callsign.length() > 0) {
			isOk = true;
		}
		else {
			std::cout << "Repeater " << i << " has an empty callsign" << std::endl ;
		}
		
		if (isOk && !isalpha(repeater->band[0])) {
			isOk = false;
			std::cout << "Repeater " << i << " band is not a letter" << std::endl;
		}

		if (isOk && repeater->address.length() == 0) {
			isOk = false;
		}

		if(!isOk) {
			delete repeater;
		} else {
			std::cout << "REPEATER: " << repeater->callsign << "-" << repeater->band << " " << repeater->address << ":" << repeater->port << std::endl;
			m_repeaters.push_back(repeater);
		}
	}

	return m_repeaters.size() > 0;
}

bool CDStarGatewayConfig::loadIrcDDB(const Config & cfg)
{
	//ircDDB Networks
	for(int i = 0; i < cfg.lookup("ircddb").getLength(); i++) {
		TircDDB * ircddb = new TircDDB();
		std::stringstream key;
		key << "ircddb.[" << i << "].hostname";
		if(! get_value(cfg, key.str(), ircddb->hostname, 5, 30, "") || ircddb->hostname == "") {//do not allow hostname to be empty
			delete ircddb;
			continue;
		}

		key.str("");key.clear();
		key << "ircddb.[" << i << "].username";
		if (! get_value(cfg, key.str(), ircddb->username, 3, 8, m_gateway.callsign)) {//default user name to callsign
			delete ircddb;
			continue;
		}
		CUtils::ToUpper(ircddb->username);

		key.str("");key.clear();
		key << "ircddb.[" << i << "].password";
		if(!get_value(cfg, key.str(), ircddb->password, 0, 30, "")) {
			delete ircddb;
			continue;
		}

		ircddb->isQuadNet = ircddb->hostname.find("openquad.net") != std::string::npos;
		this->m_ircDDB.push_back(ircddb);
		std::cout << "IRCDDB: host=" << ircddb->hostname << " user=" << ircddb->username << " password=" << ircddb->password << std::endl;
	}

	if(this->m_ircDDB.size() == 0) {//no ircddb network specified? Default to openquad!
		TircDDB * ircddb  = new TircDDB();
		ircddb->hostname  = "ipv4.openquad.net";
		ircddb->password  = "";
		ircddb->username  = m_gateway.callsign;
		ircddb->isQuadNet = true;
		this->m_ircDDB.push_back(ircddb);
		std::cout << "No ircDDB networks configured, defaulting to IRCDDB: host=" << ircddb->hostname << " user=" << ircddb->username << " password=" << ircddb->password << "\n";
	}

	return true;
}

bool CDStarGatewayConfig::loadGateway(const Config & cfg)
{
	if (! get_value(cfg, "gateway.callsign", m_gateway.callsign, 3, 8, "")
	|| 0 == m_gateway.callsign.size())
	{
		printf("CONFIG: No gateway callsign specified");
		return false;
	}
	CUtils::ToUpper(m_gateway.callsign);
	get_value(cfg, "gateway.address", m_gateway.address, 0, 20, "0.0.0.0", true);
	get_value(cfg, "gateway.hbaddress", m_gateway.hbAddress, 0, 20, "127.0.0.1", true);
	get_value(cfg, "gateway.hbport", m_gateway.hbPort, 1U, 65535U, 20010U);
	get_value(cfg, "gateway.icomAddress", m_gateway.icomAddress, 0, 20, "127.0.0.1", true);
	get_value(cfg, "gateway.icomPort", m_gateway.icomPort, 1U, 65535U, 20000U);
	get_value(cfg, "gateway.latitude", m_gateway.latitude, -90.0, 90.0, 0.0);
	get_value(cfg, "gateway.longitude", m_gateway.longitude, -180.0, 180.0, 0.0);
	get_value(cfg, "gateway.description1", m_gateway.description1, 0, 1024, "");
	get_value(cfg, "gateway.description2", m_gateway.description2, 0, 1024, "");
	get_value(cfg, "gateway.url", m_gateway.url, 0, 1024, "");

	std::cout << "GATEWAY: callsign='" << m_gateway.callsign << "' listen address='" << m_gateway.address << std::endl;

	CUtils::clean(m_gateway.description1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.description2, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.url, 		  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");

	//properly size values
	m_gateway.callsign.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
	m_gateway.callsign.push_back('G');

	return true;
}

bool CDStarGatewayConfig::open(Config & cfg)
{
	if (m_fileName.size() < 1) {
		printf("Configuration filename too short!\n");
		return false;
	}

	try {
		cfg.readFile(m_fileName.c_str());
	}
	catch(const FileIOException &fioex) {
		printf("Can't read %s\n", m_fileName.c_str());
		return false;
	}
	catch(const ParseException &pex) {
		printf("Parse error at %s:%d - %s\n", pex.getFile(), pex.getLine(), pex.getError());
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

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, unsigned int &value, unsigned int min, unsigned int max, unsigned int default_value)
{
	value = default_value;
	int valueTmp;
	if(get_value(cfg, path, valueTmp, (int)min, (int)max, (int)default_value)
		&& valueTmp >= 0) {
			value = valueTmp;
	}

	return true;
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, int &value, int min, int max, int default_value)
{
	if (cfg.lookupValue(path, value)) {
		if (value < min || value > max)
			value = default_value;
	} else
		value = default_value;
	return true;
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, double &value, double min, double max, double default_value)
{
	if (cfg.lookupValue(path, value)) {
		if (value < min || value > max)
			value = default_value;
	} else
		value = default_value;
	return true;
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, bool &value, bool default_value)
{
	if (! cfg.lookupValue(path, value))
		value = default_value;
	return true;
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value)
{
	return get_value(cfg, path, value, min, max, default_value, false);
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value, bool emptyToDefault)
{
	if (cfg.lookupValue(path, value)) {
		int l = value.length();
		if (l<min || l>max) {
			std::cout << path << "=" << value << " has an inalid length, must be between " << min << " and " <<  max << " actual " << l << "\n";
			return false;
		}
	} else
		value = default_value;

	if(emptyToDefault && value.length() == 0) {
		value = default_value;
	}

	return true;
}

bool CDStarGatewayConfig::get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value, bool emptyToDefault, const std::vector<std::string>& allowedValues)
{
	bool ret = get_value(cfg, path, value, min, max, default_value, emptyToDefault);

	if(ret) {
		for(std::string s : allowedValues) {
			if(s == value) {
				ret = true;
				break;
			}
		}
	}

	if(!ret) {
		std::stringstream message;
		message << path << "=" << value << " has an invalid value. Valid values are : ";
		for(std::string s : allowedValues) {
			message << s << ", ";
		}
		message << std::endl;
		std::cout << message.str();
	}

	return ret;
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

void CDStarGatewayConfig::getPaths(Tpaths & paths) const
{
	paths = m_paths;
}
