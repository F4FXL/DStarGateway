/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
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

#include <string>
#include <iostream>
#include <cassert>
#include <sstream>
#ifdef DEBUG_DSTARGW
#include <boost/stacktrace.hpp>
#endif

#include "DGWTimeServerApp.h"
#include "Version.h"
#include "Log.h"
#include "Daemon.h"

CDGWTimeServerApp * CDGWTimeServerApp::g_app = nullptr;
const std::string BANNER_1 = CStringUtils::string_format("%s v%s Copyright (C) %s\n", APPLICATION_NAME.c_str(), LONG_VERSION.c_str(),  VENDOR_NAME.c_str());
const std::string BANNER_2 = "DGWTimeServer comes with ABSOLUTELY NO WARRANTY; see the LICENSE for details.\n";
const std::string BANNER_3 = "This is free software, and you are welcome to distribute it under certain conditions that are discussed in the LICENSE file.\n\n";

int main(int argc, char * argv[])
{
	std::set_terminate(CDGWTimeServerApp::terminateHandler);

	signal(SIGSEGV, CDGWTimeServerApp::sigHandlerFatal);
	signal(SIGILL, CDGWTimeServerApp::sigHandlerFatal);
	signal(SIGFPE, CDGWTimeServerApp::sigHandlerFatal);
	signal(SIGABRT, CDGWTimeServerApp::sigHandlerFatal);
	signal(SIGTERM, CDGWTimeServerApp::sigHandler);
	signal(SIGINT, CDGWTimeServerApp::sigHandler);

    if (2 != argc) {
		printf("usage: %s path_to_config_file\n", argv[0]);
		printf("       %s --version\n", argv[0]);
		return 1;
	}

	std::cout << std::endl << BANNER_1 << BANNER_2 << BANNER_3;
	if(argv[1][0] == '-') {
		return 0;
	}

	std::string configfile(argv[1]);
	CTimeServerConfig config(configfile);
	if(!config.load())
		return 1;

	TDaemon daemon;
	config.getDameon(daemon);
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
				CLog::finalise();
				return 1;
		}
	}

	CDGWTimeServerApp app(&config);

	if(!app.init())
		return 0;

	app.run();

	return 0;
}

CDGWTimeServerApp::CDGWTimeServerApp(const CTimeServerConfig * config) :
m_config(config)
{
	g_app = this;
	assert(config != nullptr);
}

CDGWTimeServerApp::~CDGWTimeServerApp()
{
	delete m_thread;
}

bool CDGWTimeServerApp::init()
{
	return createThread();
}

void CDGWTimeServerApp::run()
{
	m_thread->Run();
	m_thread->Wait();
}

bool CDGWTimeServerApp::createThread()
{
	m_thread = new CTimeServerThread();

	TTimeServer timeserver;
	m_config->getTimeServer(timeserver);

	std::vector<std::string> rptrs = { "", "", "", "" };
	TRepeater repeater;
	for(unsigned int i = 0u; i < m_config->getRepeaterCount(); i++) {
		m_config->getRepeater(i, repeater);
		rptrs[i].assign(repeater.band);
	}

	TPaths paths;
	m_config->getPaths(paths);

	m_thread = new CTimeServerThread();
	bool ret = m_thread->setGateway(timeserver.callsign, rptrs[0], rptrs[1], rptrs[2], rptrs[3], timeserver.address, paths.data);
	if(ret) {
		m_thread->setAnnouncements(timeserver.language, timeserver.format, timeserver.interval);
	}

	return ret;
}

void CDGWTimeServerApp::sigHandler(int sig)
{
	CLog::logInfo("Caught signal : %s, shutting down gateway", strsignal(sig));

	if(g_app != nullptr && g_app->m_thread != nullptr) {
		g_app->m_thread->kill();
	}
}

void CDGWTimeServerApp::sigHandlerFatal(int sig)
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

void CDGWTimeServerApp::terminateHandler()
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