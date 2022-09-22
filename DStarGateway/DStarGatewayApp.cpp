/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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
#include <signal.h>
#include <exception>
#include <cassert>
#include <unistd.h>
#ifdef DEBUG_DSTARGW
#include <boost/stacktrace.hpp>
#endif

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
#include "Daemon.h"

CDStarGatewayApp * CDStarGatewayApp::g_app = nullptr;
const std::string BANNER_1 = CStringUtils::string_format("%s Copyright (C) %s\n", FULL_PRODUCT_NAME.c_str(), VENDOR_NAME.c_str());
const std::string BANNER_2 = "DStarGateway comes with ABSOLUTELY NO WARRANTY; see the LICENSE for details.\n";
const std::string BANNER_3 = "This is free software, and you are welcome to distribute it under certain conditions that are discussed in the LICENSE file.\n\n";

#ifdef UNIT_TESTS
int fakemain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	std::set_terminate(CDStarGatewayApp::terminateHandler);

	signal(SIGSEGV, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGILL, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGFPE, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGABRT, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGTERM, CDStarGatewayApp::sigHandler);
	signal(SIGINT, CDStarGatewayApp::sigHandler);

	setbuf(stdout, NULL);
	if (2 != argc) {
		printf("usage: %s path_to_config_file\n", argv[0]);
		printf("       %s --version\n", argv[0]);
		return 1;
	}

	std::cout << std::endl << BANNER_1 << BANNER_2 << BANNER_3;

	if ('-' == argv[1][0]) {
		return 0;
	}

	CDStarGatewayConfig * config = new CDStarGatewayConfig(std::string((argv[1])));
	if(!config->load()) {
		CLog::logFatal("Invalid configuration, aborting");
		return false;
	}

	TDaemon daemon;
	config->getDaemon(daemon);

	if (daemon.daemon) {
		CLog::logInfo("Configured as a daemon, detaching ...");
		auto res = CDaemon::daemonise(daemon.pidFile, daemon.user);

		switch (res)
		{
			case DR_PARENT:
				return 0;
			case DR_CHILD:
				break;
			case DR_PIDFILE_FAILED:
			case DR_FAILURE:
			default:
				CLog::logFatal("Failed to run as daemon");
				delete config;
				CLog::finalise();
				return 1;
		}
	}

	// Setup Log
	TLog logConf;
	config->getLog(logConf);
	CLog::finalise();
	if(logConf.displayLevel	!= LOG_NONE && !daemon.daemon) CLog::addTarget(new CLogConsoleTarget(logConf.displayLevel));
	if(logConf.fileLevel		!= LOG_NONE) CLog::addTarget(new CLogFileTarget(logConf.fileLevel, logConf.logDir, logConf.fileRoot, logConf.fileRotate));

	//write banner in log file if we are dameon
	if(daemon.daemon) {
		CLog::logInfo(BANNER_1);
		CLog::logInfo(BANNER_2);
		CLog::logInfo(BANNER_3);
	}

	CDStarGatewayApp gateway(config);
	
	if (!gateway.init()) {
		return 1;
	}

	gateway.run();

	if(daemon.daemon) {
		CDaemon::finalise();
	}

	return 0;
}

CDStarGatewayApp::CDStarGatewayApp(CDStarGatewayConfig * config) :
m_config(config),
m_thread(NULL)
{
	assert(config != nullptr);
	g_app = this;
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
	// Log
	TLog log;
	m_config->getLog(log);

	// Paths
	Tpaths paths;
	m_config->getPaths(paths);
	m_thread = new CDStarGatewayThread(log.logDir, paths.dataDir, "");

	// Setup the gateway
	TGateway gatewayConfig;
	m_config->getGateway(gatewayConfig);
	m_thread->setGateway(gatewayConfig.type, gatewayConfig.callsign, gatewayConfig.address);
	m_thread->setLanguage(gatewayConfig.language);
	m_thread->setLocation(gatewayConfig.latitude, gatewayConfig.longitude);

#ifdef USE_GPSD
	// Setup GPSD
	TGPSD gpsdConfig;
	m_config->getGPSD(gpsdConfig);
#endif

	// Setup APRS
	TAPRS aprsConfig;
	m_config->getAPRS(aprsConfig);
	CAPRSHandler * aprsWriter = NULL;
	if(aprsConfig.enabled && !aprsConfig.password.empty()) {
		aprsWriter = new CAPRSHandler(aprsConfig.hostname, aprsConfig.port, gatewayConfig.callsign, aprsConfig.password, gatewayConfig.address);
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

	// Setup access control
	TAccessControl accessControl;
	m_config->getAccessControl(accessControl);

	CCallsignList * whiteList = new CCallsignList(accessControl.whiteList);
	if(whiteList->load() && whiteList->getCount() > 0U) {
		m_thread->setWhiteList(whiteList);
	}
	else {
		delete whiteList;
	}

	CCallsignList * blackList = new CCallsignList(accessControl.blackList);
	if(blackList->load() && blackList->getCount() > 0U) {
		m_thread->setBlackList(blackList);
	}
	else {
		delete blackList;
	}

	CCallsignList * restrictList = new CCallsignList(accessControl.restrictList);
	if(restrictList->load() && restrictList->getCount() > 0U) {
		m_thread->setRestrictList(restrictList);
	}
	else {
		delete restrictList;
	}

	// Drats
	TDRats drats;
	m_config->getDRats(drats);

	// Setup the repeaters
	bool ddEnabled = false;
	bool atLeastOneRepeater = false;
	CRepeaterProtocolHandlerFactory repeaterProtocolFactory;
	for(unsigned int i = 0U; i < m_config->getRepeaterCount(); i++) {
		TRepeater rptrConfig;
		m_config->getRepeater(i, rptrConfig);
		auto  repeaterProtocolHandler = repeaterProtocolFactory.getRepeaterProtocolHandler(rptrConfig.hwType, gatewayConfig, rptrConfig.address, rptrConfig.port);
		if(repeaterProtocolHandler == nullptr)
			continue;
		atLeastOneRepeater = true;
		m_thread->addRepeater(rptrConfig.callsign,
								rptrConfig.band,
								rptrConfig.address,
								rptrConfig.port,
								rptrConfig.hwType,
								rptrConfig.reflector,
								rptrConfig.reflectorAtStartup,
								rptrConfig.reflectorReconnect,
								rptrConfig.frequency,
								drats.enabled,
								rptrConfig.offset,
								rptrConfig.range,
								rptrConfig.latitude,
								rptrConfig.longitude,
								rptrConfig.agl,
								rptrConfig.description1,
								rptrConfig.description2,
								rptrConfig.url,
								repeaterProtocolHandler,
								rptrConfig.band1,
								rptrConfig.band2,
								rptrConfig.band3);

		aprsWriter->setPort(rptrConfig.callsign, rptrConfig.band, rptrConfig.frequency, rptrConfig.offset, rptrConfig.range, rptrConfig.latitude, rptrConfig.longitude, rptrConfig.agl);

		if(!ddEnabled) ddEnabled = rptrConfig.band.length() > 1U;
	}

	if(!atLeastOneRepeater) {
		CLog::logError("Error: no repeaters are enabled or opening network communication to repeater failed");
		return false;
	}

	m_thread->setDDModeEnabled(ddEnabled);
	CLog::logInfo("DD Mode enabled: %d", int(ddEnabled));

	// Setup ircddb
	auto ircddbVersionInfo = "linux_" + PRODUCT_NAME + "-" + VERSION;
	std::vector<CIRCDDB *> clients;
	for(unsigned int i=0; i < m_config->getIrcDDBCount(); i++) {
		TircDDB ircDDBConfig;
		m_config->getIrcDDB(i, ircDDBConfig);
		CLog::logInfo("ircDDB Network %d set to %s user: %s, Quadnet %d", i + 1,ircDDBConfig.hostname.c_str(), ircDDBConfig.username.c_str(), ircDDBConfig.isQuadNet);
		CIRCDDB * ircDDB = new CIRCDDBClient(ircDDBConfig.hostname, 9007U, ircDDBConfig.username, ircDDBConfig.password, ircddbVersionInfo, gatewayConfig.address, ircDDBConfig.isQuadNet);
		clients.push_back(ircDDB);
	}
	if(clients.size() > 0U) {
		CIRCDDBMultiClient* multiClient = new CIRCDDBMultiClient(clients);
		bool res = multiClient->open();
		if (!res) {
			CLog::logInfo("Cannot initialise the ircDDB protocol handler\n");
			return false;
		}
		m_thread->setIRC(multiClient);
	}

	// Setup Dextra
	TDextra dextraConfig;
	m_config->getDExtra(dextraConfig);
	CLog::logInfo("DExtra enabled: %d, max. dongles: %u", int(dextraConfig.enabled), dextraConfig.maxDongles);
	m_thread->setDExtra(dextraConfig.enabled, dextraConfig.maxDongles);

	// Setup DCS
	TDCS dcsConfig;
	m_config->getDCS(dcsConfig);
	CLog::logInfo("DCS enabled: %d", int(dcsConfig.enabled));
	m_thread->setDCS(dcsConfig.enabled);

	// Setup DPlus
	TDplus dplusConfig;
	m_config->getDPlus(dplusConfig);
	CLog::logInfo("D-Plus enabled: %d, max. dongles: %u, login: %s", int(dplusConfig.enabled), dplusConfig.maxDongles, dplusConfig.login.c_str());
	m_thread->setDPlus(dplusConfig.enabled, dplusConfig.maxDongles, dplusConfig.login);

	// Setup XLX
	TXLX xlxConfig;
	m_config->getXLX(xlxConfig);
	CLog::logInfo("XLX enabled: %d, Hosts file url: %s", int(xlxConfig.enabled), xlxConfig.url.c_str());
	m_thread->setXLX(xlxConfig.enabled, xlxConfig.enabled ? CXLXHostsFileDownloader::download(xlxConfig.url) : "");

	// Setup Remote
	TRemote remoteConfig;
	m_config->getRemote(remoteConfig);
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

void CDStarGatewayApp::sigHandler(int sig)
{
	CLog::logInfo("Caught signal : %s, shutting down gateway", strsignal(sig));

	if(g_app != nullptr && g_app->m_thread != nullptr) {
		g_app->m_thread->kill();
	}
}

void CDStarGatewayApp::sigHandlerFatal(int sig)
{
	CLog::logFatal("Caught signal : %s", strsignal(sig));
	fprintf(stderr, "Caught signal : %s\n", strsignal(sig));
#ifdef DEBUG_DSTARGW
	std::stringstream stackTrace;
	stackTrace <<  boost::stacktrace::stacktrace();
	CLog::logFatal("Stack Trace : \n%s", stackTrace.str().c_str());
	fprintf(stderr, "Stack Trace : \n%s\n", stackTrace.str().c_str());
#endif
	exit(3);
}

void CDStarGatewayApp::terminateHandler()
{
#ifdef DEBUG_DSTARGW
	std::stringstream stackTrace;
	stackTrace <<  boost::stacktrace::stacktrace();
#endif

	std::exception_ptr eptr;
	eptr = std::current_exception(); 

	try {
        if (eptr != nullptr) {
            std::rethrow_exception(eptr);
        }
		else {
			CLog::logFatal("Unhandled unknown exception occured");
			fprintf(stderr, "Unknown ex\n");
		}
    } catch(const std::exception& e) {
        CLog::logFatal("Unhandled exception occured %s", e.what());
		fprintf(stderr, "Unhandled ex %s\n", e.what());
    }

#ifdef DEBUG_DSTARGW
	CLog::logFatal("Stack Trace : \n%s", stackTrace.str().c_str());
#endif
	exit(2);
}
