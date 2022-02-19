/*
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "TimeServerConfig.h"
#include "Log.h"
#include "StringUtils.h"

CTimeServerConfig::CTimeServerConfig(const std::string &pathname) :
m_fileName(pathname),
m_repeaters()
{

}

CTimeServerConfig::~CTimeServerConfig()
{
	for(auto repeater : m_repeaters) {
		delete repeater;
	}
	m_repeaters.clear();
}

bool CTimeServerConfig::load()
{
	bool ret = false;
	CLog::logInfo("Loading Configuration from %s", m_fileName.c_str());
	CConfig cfg(m_fileName);

	ret = open(cfg);
	if(ret) {
		ret = loadTimeServer(cfg) && ret;
		ret = loadRepeaters(cfg) && ret;
        ret = loadDaemon(cfg) && ret;
		ret = loadPaths(cfg) && ret;
    }

	return ret;
}

bool CTimeServerConfig::open(CConfig & cfg)
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

bool CTimeServerConfig::loadRepeaters(const CConfig & cfg)
{
	m_repeaters.clear();

	for(unsigned int i = 0; i < 4; i++) {
		std::string section = CStringUtils::string_format("repeater_%d", i+ 1);
		bool repeaterEnabled;

		bool ret = cfg.getValue(section, "enabled", repeaterEnabled, false);
		if(!ret || !repeaterEnabled)
			continue;

		TRepeater * repeater = new TRepeater;
		ret = cfg.getValue(section, "band", repeater->band, 1, 2, "") && ret;

		bool alreadyConfigured = std::any_of(m_repeaters.begin(), m_repeaters.end(), [repeater](TRepeater * r) { return r->band == repeater->band;});
		if(alreadyConfigured) {
			CLog::logWarning("%s-%s repeater already configured, ignoring", m_timeServer.callsign.c_str(), repeater->band.c_str());
			delete repeater;
			continue;
		}

		m_repeaters.push_back(repeater);
	}

	return m_repeaters.size() > 0U;
}

bool CTimeServerConfig::loadTimeServer(const CConfig & cfg)
{
    bool ret = cfg.getValue("timeserver", "callsign", m_timeServer.callsign, 3, 8, "");
	boost::to_upper(m_timeServer.callsign);
    ret = cfg.getValue("timeserver", "address", m_timeServer.address, 0, 1024, "127.0.0.1") && ret;

	std::string format;
	ret = cfg.getValue("timeserver", "format", format, "voiceandtext",  {"voice", "text", "voiceandtext"}) && ret;
	if(format == "voice")				m_timeServer.format = FORMAT_VOICE_TIME;
	else if(format == "text")			m_timeServer.format = FORMAT_TEXT_TIME;
	else if(format == "voiceandtext")	m_timeServer.format = FORMAT_VOICE_ALL;

	std::string lang;
	ret = cfg.getValue("timeserver", "language", lang, "english_uk_1", {"english_uk_1", "english_uk_2", "english_us_1", "english_us_2", "deutsch_1", "deutsch_2", "francais", "nederlands", "svenska", "espanol", "norsk", "portugues"}) && ret;;
	if     (lang == "english_uk_1") m_timeServer.language = LANG_ENGLISH_UK_1;
	else if(lang == "english_uk_2") m_timeServer.language = LANG_ENGLISH_UK_2;
	else if(lang == "english_us_1") m_timeServer.language = LANG_ENGLISH_US_1;
	else if(lang == "english_us_2") m_timeServer.language = LANG_ENGLISH_US_2;
	else if(lang == "deutsch_1"   ) m_timeServer.language = LANG_DEUTSCH_1;
	else if(lang == "detusch_2"   ) m_timeServer.language = LANG_DEUTSCH_2;
	else if(lang == "francais"    ) m_timeServer.language = LANG_FRANCAIS;
	else if(lang == "nederlands"  ) m_timeServer.language = LANG_NEDERLANDS;
	else if(lang == "svenska"     ) m_timeServer.language = LANG_SVENSKA;
	else if(lang == "espanol"     ) m_timeServer.language = LANG_ESPANOL;
	else if(lang == "norsk"       ) m_timeServer.language = LANG_NORSK;
	else if(lang == "portugues"   ) m_timeServer.language = LANG_PORTUGUES;

	std::string interval;
	ret = cfg.getValue("timeserver", "interval", interval, "30", {"15", "30", "60"}) && ret;
	if(interval == "15")		m_timeServer.interval = INTERVAL_15MINS;
	else if(interval == "30")	m_timeServer.interval = INTERVAL_30MINS;
	else if(interval == "60")	m_timeServer.interval = INTERVAL_60MINS;

	return ret;
}

bool CTimeServerConfig::loadDaemon(const CConfig & cfg)
{
	bool ret = cfg.getValue("daemon", "daemon", m_daemon.daemon, false);
	ret = cfg.getValue("daemon", "pidfile", m_daemon.pidFile, 0, 1024, "") && ret;
	ret = cfg.getValue("daemon", "user", m_daemon.user, 0, 1024, "") && ret;
	return ret;
}

bool CTimeServerConfig::loadPaths(const CConfig & cfg)
{
	bool ret = cfg.getValue("paths", "data", m_paths.data, 1, 1024, "");
	return ret;
}

void CTimeServerConfig::getTimeServer(TTimeServer& timeserver) const
{
	timeserver = m_timeServer;
}

void CTimeServerConfig::getDameon(TDaemon& daemon) const
{
	daemon = m_daemon;
}

unsigned int CTimeServerConfig::getRepeaterCount() const
{
	return m_repeaters.size();
} 

void CTimeServerConfig::getRepeater(unsigned int idx, TRepeater& repeater) const
{
	repeater = *(m_repeaters[idx]);
}

void CTimeServerConfig::getPaths(TPaths& paths) const
{
	paths = m_paths;
}
