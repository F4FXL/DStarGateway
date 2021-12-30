/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <vector>

#include "DStarGatewayDefs.h"
#include "DStarGatewayConfig.h"
#include "DStarGatewayApp.h"
#include "Version.h"
#include "IRCDDBMultiClient.h"
#include "IRCDDBClient.h"
#include "Utils.h"
#include "Version.h"
#include "GitVersion.h"
#include "RepeaterProtocolHandlerFactory.h"
#include "XLXHostsFileDownloader.h"
#include "Log.h"
#include "LogFileTarget.h"
#include "LogConsoleTarget.h"
#include "APRSGPSDIdFrameProvider.h"
#include "APRSFixedIdFrameProvider.h"

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	if (2 != argc) {
		printf("usage: %s path_to_config_file\n", argv[0]);
		printf("       %s --version\n", argv[0]);
		return 1;
	}

	if ('-' == argv[1][0]) {
		printf("\n%s Copyright (C) %s\n", FULL_PRODUCT_NAME.c_str(), VENDOR_NAME.c_str());
		printf("DStarGateway comes with ABSOLUTELY NO WARRANTY; see the LICENSE for details.\n");
		printf("This is free software, and you are welcome to distribute it\nunder certain conditions that are discussed in the LICENSE file.\n\n");
		return 0;
	}

	std::string cfgFile(argv[1]);

	CDStarGatewayApp gateway(cfgFile);
	
	if (!gateway.init()) {
		return 1;
	}

	gateway.run();

	return 0;
}

CDStarGatewayApp::CDStarGatewayApp(const std::string &configFile) : m_configFile(configFile), m_thread(NULL)
{
}

CDStarGatewayApp::~CDStarGatewayApp()
{
}

bool CDStarGatewayApp::init()
{
	return createThread();
}

void CDStarGatewayApp::run()
{
	m_thread->Run();
	m_thread->Wait();
	CLog::logInfo("exiting\n");
	CLog::finalise();
}

bool CDStarGatewayApp::createThread()
{
	printf("\n%s Copyright (C) %s\n", FULL_PRODUCT_NAME.c_str(), VENDOR_NAME.c_str());
	printf("DStarGateway comes with ABSOLUTELY NO WARRANTY; see the LICENSE for details.\n");
	printf("This is free software, and you are welcome to distribute it\nunder certain conditions that are discussed in the LICENSE file.\n\n");

	CDStarGatewayConfig config(m_configFile);
	if(!config.load()) {
		CLog::logFatal("Invalid configuration, aborting");
		return false;
	}

	// Setup Log
	TLog log;
	config.getLog(log);
	CLog::finalise();
	if(log.m_displayLevel	!= LOG_NONE) CLog::addTarget(new CLogConsoleTarget(log.m_displayLevel));
	if(log.m_fileLevel		!= LOG_NONE) CLog::addTarget(new CLogFileTarget(log.m_fileLevel, log.logDir, log.m_fileRotate));

	Tpaths paths;
	config.getPaths(paths);
	m_thread = new CDStarGatewayThread(log.logDir, paths.dataDir, "");

	// Setup the gateway
	TGateway gatewayConfig;
	config.getGateway(gatewayConfig);
	m_thread->setGateway(gatewayConfig.type, gatewayConfig.callsign, gatewayConfig.address);
	m_thread->setLanguage(gatewayConfig.language);
	m_thread->setLocation(gatewayConfig.latitude, gatewayConfig.longitude);

#ifdef USE_GPSD
	// Setup GPSD
	TGPSD gpsdConfig;
	config.getGPSD(gpsdConfig);
#endif

	// Setup APRS
	TAPRS aprsConfig;
	config.getAPRS(aprsConfig);
	CAPRSWriter * aprsWriter = NULL;
	if(aprsConfig.enabled && !aprsConfig.password.empty()) {
		aprsWriter = new CAPRSWriter(aprsConfig.hostname, aprsConfig.port, gatewayConfig.callsign, aprsConfig.password, gatewayConfig.address);
		if(aprsWriter->open()) {
#ifdef USE_GPSD
			CAPRSIdFrameProvider * idFrameProvider = aprsConfig.m_positionSource == POSSRC_GPSD ? (CAPRSIdFrameProvider *)new CAPRSGPSDIdFrameProvider(gpsdConfig.m_address, gpsdConfig.m_port)
																									: new CAPRSFixedIdFrameProvider();
#else
			CAPRSIdFrameProvider * idFrameProvider = new CAPRSFixedIdFrameProvider();
#endif
			idFrameProvider->start();
			aprsWriter->setIdFrameProvider(idFrameProvider);
			m_thread->setAPRSWriter(aprsWriter);
		}
		else {
			delete aprsWriter;
			aprsWriter = NULL;
		}
	}

	// Setup the repeaters
	if(config.getRepeaterCount() == 0U) {
		CLog::logInfo("No repeater configured\n");
		return false;
	}
	bool ddEnabled = false;
	CRepeaterProtocolHandlerFactory repeaterProtocolFactory;
	for(unsigned int i = 0U; i < config.getRepeaterCount(); i++) {
		TRepeater rptrConfig;
		config.getRepeater(i, rptrConfig);
		m_thread->addRepeater(rptrConfig.callsign,
								rptrConfig.band,
								rptrConfig.address,
								rptrConfig.port,
								rptrConfig.hwType,
								rptrConfig.reflector,
								rptrConfig.reflectorAtStartup,
								rptrConfig.reflectorReconnect,
								rptrConfig.frequency,
								rptrConfig.offset,
								rptrConfig.range,
								rptrConfig.latitude,
								rptrConfig.longitude,
								rptrConfig.agl,
								rptrConfig.description1,
								rptrConfig.description2,
								rptrConfig.url,
								repeaterProtocolFactory.getRepeaterProtocolHandler(rptrConfig.hwType, gatewayConfig, rptrConfig.address, rptrConfig.port),
								rptrConfig.band1,
								rptrConfig.band2,
								rptrConfig.band3);

		aprsWriter->setPortFixed(rptrConfig.callsign, rptrConfig.band, rptrConfig.frequency, rptrConfig.offset, rptrConfig.range, rptrConfig.latitude, rptrConfig.longitude, rptrConfig.agl);

		if(!ddEnabled) ddEnabled = rptrConfig.band.length() > 1U;
	}
	m_thread->setDDModeEnabled(ddEnabled);
	CLog::logInfo("DD Mode enabled: %d", int(ddEnabled));

	// Setup ircddb
	std::vector<CIRCDDB *> clients;
	for(unsigned int i=0; i < config.getIrcDDBCount(); i++) {
		TircDDB ircDDBConfig;
		config.getIrcDDB(i, ircDDBConfig);
		CLog::logInfo("ircDDB Network %d set to %s user: %s, Quadnet %d", i + 1,ircDDBConfig.hostname.c_str(), ircDDBConfig.username.c_str(), ircDDBConfig.isQuadNet);
		CIRCDDB * ircDDB = new CIRCDDBClient(ircDDBConfig.hostname, 9007U, ircDDBConfig.username, ircDDBConfig.password, FULL_PRODUCT_NAME, gatewayConfig.address, ircDDBConfig.isQuadNet);
		clients.push_back(ircDDB);
	}
	CIRCDDBMultiClient* multiClient = new CIRCDDBMultiClient(clients);
	bool res = multiClient->open();
	if (!res) {
		CLog::logInfo("Cannot initialise the ircDDB protocol handler\n");
		return false;
	}
	m_thread->setIRC(multiClient);

	// Setup Dextra
	TDextra dextraConfig;
	config.getDExtra(dextraConfig);
	CLog::logInfo("DExtra enabled: %d, max. dongles: %u", int(dextraConfig.enabled), dextraConfig.maxDongles);
	m_thread->setDExtra(dextraConfig.enabled, dextraConfig.maxDongles);

	// Setup DCS
	TDCS dcsConfig;
	config.getDCS(dcsConfig);
	CLog::logInfo("DCS enabled: %d", int(dcsConfig.enabled));
	m_thread->setDCS(dcsConfig.enabled);

	// Setup DPlus
	TDplus dplusConfig;
	config.getDPlus(dplusConfig);
	CLog::logInfo("D-Plus enabled: %d, max. dongles: %u, login: %s", int(dplusConfig.enabled), dplusConfig.maxDongles, dplusConfig.login.c_str());
	m_thread->setDPlus(dplusConfig.enabled, dplusConfig.maxDongles, dplusConfig.login);

	// Setup XLX
	TXLX xlxConfig;
	config.getXLX(xlxConfig);
	CLog::logInfo("XLX enabled: %d, Hosts file url: %s", int(xlxConfig.enabled), xlxConfig.url.c_str());
	m_thread->setXLX(xlxConfig.enabled, xlxConfig.enabled ? CXLXHostsFileDownloader::download(xlxConfig.url) : "");

	// Setup Remote
	TRemote remoteConfig;
	config.getRemote(remoteConfig);
	CLog::logInfo("Remote enabled: %d, port %u", int(remoteConfig.enabled), remoteConfig.port);
	m_thread->setRemote(remoteConfig.enabled, remoteConfig.password, remoteConfig.port);

	// Get final things ready
	m_thread->setIcomRepeaterHandler(repeaterProtocolFactory.getIcomProtocolHandler());
	m_thread->setHBRepeaterHandler(repeaterProtocolFactory.getHBProtocolHandler());
	m_thread->setDummyRepeaterHandler(repeaterProtocolFactory.getDummyProtocolHandler());
	m_thread->setInfoEnabled(true);
	m_thread->setEchoEnabled(true);
	m_thread->setDTMFEnabled(true);
	m_thread->setLog(true);

	return true;
}

