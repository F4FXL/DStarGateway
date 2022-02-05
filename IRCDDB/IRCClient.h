/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2011,2012   Jonathan Naylor, G4KLX
Copyright (c) 2017 by Thomas A. Early N7TAE

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <future>

#include "IRCReceiver.h"
#include "IRCMessageQueue.h"
#include "IRCProtocol.h"
#include "IRCApplication.h"

class IRCClient 
{
public:
	IRCClient( IRCApplication *app, const std::string& update_channel, const std::string& hostName, unsigned int port, const std::string& callsign,
															const std::string& password, const std::string& versionInfo, const std::string& localAddr);
	~IRCClient();
	void startWork();
	void stopWork();

protected:
	void Entry();

private:
	char m_host_name[100];
	char m_local_addr[100];
	unsigned int m_port;
	std::string m_callsign;
	std::string m_password;
	bool m_terminateThread;

	IRCReceiver *m_recv;
	IRCMessageQueue *m_recvQ;
	IRCMessageQueue *m_sendQ;
	IRCProtocol *m_proto;
	IRCApplication *m_app;
	std::future<void> m_future;
};

