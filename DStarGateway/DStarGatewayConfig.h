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
#pragma once

#include <string>
#include <vector>

#include "Defs.h"
#include "Config.h"
#include "LogSeverity.h"

typedef struct {
	bool daemon;
	std::string pidFile;
} TDaemon;

typedef struct {
	GATEWAY_TYPE type;
	std::string callsign;
	std::string address;
	std::string hbAddress;
	unsigned int hbPort;
	std::string icomAddress;
	unsigned int icomPort;
	double latitude;
	double longitude;
	std::string description1;
	std::string description2;
	std::string url; 
	TEXT_LANG language;
} TGateway;

typedef struct {
	std::string band;
	std::string callsign;
	std::string reflector;
	std::string address;
	unsigned int port;
	HW_TYPE hwType;
	bool reflectorAtStartup;
	RECONNECT reflectorReconnect;
#ifdef USE_DRATS
	bool dRatsEnabled;
#endif
	double frequency;
	double offset;
	double range;
	double latitude;
	double longitude;
	double agl;
	std::string description1;
	std::string description2;
	std::string url;
	unsigned char band1;
	unsigned char band2;
	unsigned char band3;
} TRepeater;

typedef struct {
	std::string hostname;
	std::string username;
	std::string password;
	bool isQuadNet;
} TircDDB;

typedef struct {
	std::string dataDir;
} Tpaths;

typedef struct {
	std::string logDir;
	LOG_SEVERITY m_displayLevel;
	LOG_SEVERITY m_fileLevel;
	std::string m_fileRoot;
	bool m_fileRotate;
} TLog;

typedef struct {
	bool enabled;
	std::string hostname;
	unsigned int port;
	std::string password;
	POSITION_SOURCE m_positionSource;
} TAPRS;

typedef struct {
	bool enabled;
	unsigned int maxDongles;
} TDextra;

typedef struct {
	bool enabled;
	std::string login;
	unsigned int maxDongles;
} TDplus;

typedef struct {
	bool enabled;
} TDCS;

typedef struct {
	bool enabled;
	std::string url;
} TXLX;

typedef struct {
	bool enabled;
	unsigned int port;
	std::string login;
	std::string password;
} TRemote;

#ifdef USE_GPSD
typedef struct {
	std::string m_address;
	std::string m_port;
} TGPSD;
#endif

class CDStarGatewayConfig {
public:
	CDStarGatewayConfig(const std::string &pathname);
	~CDStarGatewayConfig();

	bool load();
	void getGateway(TGateway & gateway) const;
	void getIrcDDB(unsigned int ircddbIndex, TircDDB & ircddb) const;
	unsigned int getIrcDDBCount() const;
	void getRepeater(unsigned int repeaterIndex, TRepeater & repeater) const;
	unsigned int getRepeaterCount() const;
	void getLog(TLog& log) const;
	void getPaths(Tpaths & paths) const;
	void getAPRS(TAPRS & aprs) const;
	void getDExtra(TDextra & dextra) const;
	void getDPlus(TDplus & dplus) const;
	void getDCS(TDCS & dcs) const;
	void getRemote(TRemote & remote) const;
	void getXLX(TXLX & xlx) const;
#ifdef USE_GPSD
	void getGPSD(TGPSD & gpsd) const;
#endif
	void getGeneral(TDaemon & gen) const;

private:
	bool open(CConfig & cfg);
	bool loadGateway(const CConfig & cfg);
	bool loadIrcDDB(const CConfig & cfg);
	bool loadRepeaters(const CConfig & cfg);
	bool loadLog(const CConfig & cfg);
	bool loadPaths(const CConfig & cfg);
	bool loadAPRS(const CConfig & cfg);
	bool loadDextra(const CConfig & cfg);
	bool loadDPlus(const CConfig & cfg);
	bool loadDCS(const CConfig & cfg);
	bool loadRemote(const CConfig & cfg);
	bool loadXLX(const CConfig & cfg);
#ifdef USE_GPSD
	bool loadGPSD(const CConfig & cfg);
#endif
	bool loadDaemon(const CConfig & cfg);

	std::string m_fileName;
	TGateway m_gateway;
	Tpaths m_paths;
	TAPRS m_aprs;
	TDextra m_dextra;
	TDplus m_dplus;
	TDCS m_dcs;
	TRemote m_remote;
	TXLX m_xlx;
	TLog m_log;
#ifdef USE_GPSD
	TGPSD m_gpsd;
#endif
	TDaemon m_general;

	std::vector<TRepeater *> m_repeaters;
	std::vector<TircDDB *> m_ircDDB;
};
