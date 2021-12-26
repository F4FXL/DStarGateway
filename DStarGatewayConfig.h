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

class CDStarGatewayConfig {
public:
	CDStarGatewayConfig(const std::string &pathname);
	~CDStarGatewayConfig();

	bool load();
	void getGateway(TGateway & gateway) const;
	void getIrcDDB(unsigned int ircddbIndex, TircDDB & ircddb) const;
	void getRepeater(unsigned int repeaterIndex, TRepeater & repeater) const;
	void getPaths(Tpaths & paths) const;
	unsigned int getRepeaterCount() const;
	unsigned int getIrcDDBCount() const;

private:
	bool open(Config & cfg);
	bool loadGateway(const Config & cfg);
	bool loadIrcDDB(const Config & cfg);
	bool loadRepeaters(const Config & cfg);
	bool loadPaths(const Config & cfg);
	bool get_value(const Config &cfg, const std::string &path, unsigned int &value, unsigned int min, unsigned int max, unsigned int default_value);
	bool get_value(const Config &cfg, const std::string &path, int &value, int min, int max, int default_value);
	bool get_value(const Config &cfg, const std::string &path, double &value, double min, double max, double default_value);
	bool get_value(const Config &cfg, const std::string &path, bool &value, bool default_value);
	bool get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value);
	bool get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value, bool emptyToDefault);
	bool get_value(const Config &cfg, const std::string &path, std::string &value, int min, int max, const std::string &default_value, bool emptyToDefault, const std::vector<std::string>& allowedValues);

	std::string m_fileName;
	TGateway m_gateway;
	Tpaths m_paths;
	std::vector<TRepeater *> m_repeaters;
	std::vector<TircDDB *> m_ircDDB;
};
