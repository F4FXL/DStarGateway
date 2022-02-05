/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cassert>
#include <cstring>

#include "DGWRemoteControlConfig.h"
#include "Log.h"
#include "StringUtils.h"

CDGWRemoteControlConfig::CDGWRemoteControlConfig(const std::string& fileName) :
m_fileName(fileName),
m_gateways()
{
	assert(!fileName.empty());
}

CDGWRemoteControlConfig::~CDGWRemoteControlConfig()
{
	for(auto gw : m_gateways) {
		delete gw;
	}

	m_gateways.clear();
}

bool CDGWRemoteControlConfig::load()
{
	CConfig cfg(m_fileName);
	bool ret = open(cfg);
	if(ret) {
		ret = loadGateways(cfg) && ret;
	}

	return ret;
}

bool CDGWRemoteControlConfig::loadGateways(CConfig & cfg)
{
	for(unsigned int i = 1U; i <= 4U; i++) {
		auto section = CStringUtils::string_format("gateway_%u", i);
		TRemoteGateway * gateway = new TRemoteGateway();

		bool ret = cfg.getValue(section, "name", gateway->m_name, 0U, 1024U, section);
		ret = cfg.getValue(section, "address", gateway->m_address, 0U, 1024U, "127.0.0.1") && ret;
		ret = cfg.getValue(section, "port", gateway->m_port, 1U, 65535U, 4242U) && ret;
		ret = cfg.getValue(section, "password", gateway->m_password, 0U, 1024U, "") && ret;

		if(!ret || gateway->m_password.empty()) {
			delete gateway;
			continue;
		}

		m_gateways.push_back(gateway);
	}

	return m_gateways.size() != 0;
}

bool CDGWRemoteControlConfig::getGateway(const std::string& name, TRemoteGateway& gateway)
{
	if(m_gateways.size() == 0U)
		return false;

	for(auto gw : m_gateways) {
		if(strcasecmp(name.c_str(), gw->m_name.c_str()) == 0)
		{
			gateway = *gw;
			return true;
		}
	}

	gateway = *(m_gateways[0]);

	return true;
}

bool CDGWRemoteControlConfig::open(CConfig & cfg)
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