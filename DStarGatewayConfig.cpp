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

bool CDStarGatewayConfig::loadRepeaters(Config & cfg)
{
	cfg.lookup("repeaters");
	// repeater parameters
	for (int i=0; i<cfg.lookup("repeaters").getLength(); i++) {
		std::stringstream key;
		bool isOk = false;
		TRepeater * repeater = new TRepeater({"", m_gateway.callsign, "", "127.0.0.1", 20011U});
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

bool CDStarGatewayConfig::loadIrcDDB(Config & cfg)
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

bool CDStarGatewayConfig::loadGateway(Config & cfg)
{
	if (! get_value(cfg, "gateway.callsign", m_gateway.callsign, 3, 8, "")
	|| 0 == m_gateway.callsign.size())
	{
		printf("CONFIG: No gateway callsign specified");
		return false;
	}
	CUtils::ToUpper(m_gateway.callsign);
	get_value(cfg, "gateway.address", m_gateway.address, 0, 20, "0.0.0.0", true);
	get_value(cfg, "gateway.port", m_gateway.port, 1U, 65535U, 20010U);
	std::cout << "GATEWAY: callsign='" << m_gateway.callsign << "' listen address='" << m_gateway.address << ":" << m_gateway.port << std::endl;

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

unsigned int CDStarGatewayConfig::getRepeaterCount()
{
	return m_repeaters.size();
}

unsigned int CDStarGatewayConfig::getIrcDDBCount()
{
	return m_ircDDB.size();
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

void CDStarGatewayConfig::getGateway(TGateway & gateway) const
{
	gateway.callsign = m_gateway.callsign;
	gateway.address = m_gateway.address;
	gateway.port = m_gateway.port;
}

void CDStarGatewayConfig::getIrcDDB(unsigned int ircddb, TircDDB & ircDDB) const
{
	ircDDB.hostname  = m_ircDDB[ircddb]->hostname;
	ircDDB.username  = m_ircDDB[ircddb]->username;
	ircDDB.password  = m_ircDDB[ircddb]->password;
	ircDDB.isQuadNet = m_ircDDB[ircddb]->isQuadNet;
}

void CDStarGatewayConfig::getRepeater(unsigned int index, TRepeater & repeater) const
{
	repeater.address = m_repeaters[index]->address;
	repeater.band = m_repeaters[index]->band;
	repeater.callsign = m_repeaters[index]->callsign;
	repeater.port = m_repeaters[index]->port;
	repeater.reflector = m_repeaters[index]->reflector;
}
