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

#pragma once

#include <string>
#include <vector>

#include "Config.h"
#include "TimeServerDefs.h"
#include "Log.h"

typedef struct {
    std::string callsign;
    std::string address;
    FORMAT format;
    LANGUAGE language;
    INTERVAL interval;
} TTimeServer;

typedef struct {
    std::string band;
} TRepeater;

typedef struct {
	bool daemon;
	std::string pidFile;
	std::string user;
} TDaemon;

typedef struct {
    std::string data;
} TPaths;

typedef struct {
	std::string logDir;
	LOG_SEVERITY displayLevel;
	LOG_SEVERITY fileLevel;
	std::string fileRoot;
	bool fileRotate;
} TLog;

class CTimeServerConfig
{
public:
    CTimeServerConfig(const std::string &pathname);
	~CTimeServerConfig();

	bool load();
    void getTimeServer(TTimeServer& timeserver) const;
    void getDameon(TDaemon& daemon) const;
    unsigned int getRepeaterCount() const;
    void getRepeater(unsigned int idx, TRepeater& repeater)  const;
    void getPaths(TPaths& paths) const;
    void getLog(TLog& log) const;

private:
	bool open(CConfig & cfg);
    bool loadRepeaters(const CConfig & cfg);
    bool loadTimeServer(const CConfig & cfg);
    bool loadDaemon(const CConfig & cfg);
    bool loadPaths(const CConfig & cfg);
    bool loadLog(const CConfig & cfg);

    std::string m_fileName;
    std::vector<TRepeater *> m_repeaters;
    TTimeServer m_timeServer;
    TDaemon m_daemon;
    TPaths m_paths;
    TLog m_log;
};
