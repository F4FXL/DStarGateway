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

#include "DGWTimeServerApp.h"

int main(int argc, char * argv[])
{
    if (2 != argc) {
		printf("usage: %s path_to_config_file\n", argv[0]);
		printf("       %s --version\n", argv[0]);
		return 1;
	}

	std::string configfile(argv[1]);
	CTimeServerConfig config(configfile);
	if(!config.load())
		return 1;

	CDGWTimeServerApp app(&config);

	if(!app.init())
		return 0;

	app.run();

	return 0;
}

CDGWTimeServerApp::CDGWTimeServerApp(const CTimeServerConfig * config) :
m_config(config)
{
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
