/*
 *   Copyright (C) 2010,2011,2012,2014 by Jonathan Naylor G4KLX
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
#include <vector>
#include <libconfig.h++>
#include "Defs.h"

using namespace libconfig;

typedef struct {
	std::string callsign;
	std::string address;
	unsigned int port;
} TGateway;

typedef struct {
	std::string band;
	std::string callsign;
	std::string reflector;
	std::string address;
	unsigned int port;
} TRepeater;

typedef struct {
	std::string hostname;
	std::string username;
	std::string password;
	bool isQuadNet;
} TircDDB;

class CDStarGatewayConfig {
public:
	CDStarGatewayConfig(const std::string &pathname);
	~CDStarGatewayConfig();

	bool load();
	void getGateway(TGateway & gateway) const;
	void getIrcDDB(unsigned int ircddbIndex, TircDDB & ircddb) const;
	void getRepeater(unsigned int repeaterIndex, TRepeater & repeater) const;
	unsigned int getRepeaterCount();
	unsigned int getIrcDDBCount();

private:
	bool open(Config & cfg);
	bool loadGateway(Config & cfg);
	bool loadIrcDDB(Config & cfg);
	bool loadRepeaters(Config & cfg);
	bool get_value(const Config &cfg, const std::string &path, unsigned int &value, unsigned int min, unsigned int max, unsigned int default_value);
	bool get_value(const Config &cfg, const std::string &path, int &value, int min, int max, int default_value);
	bool get_value(const Config &cfg, const std::string &path, bool &value, bool default_value);
	bool get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value);
	bool get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value, bool emptyToDefault);

	std::string m_fileName;
	TGateway m_gateway;
	std::vector<TRepeater *> m_repeaters;
	std::vector<TircDDB *> m_ircDDB;
};
