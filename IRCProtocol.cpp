/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (c) 2017 by Thomas A. Early N7TAE
Copyright (c) 2021 by Thomas Geoffrey Merck F4FXL / KC3FRA

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

#include <regex>

#include "IRCProtocol.h"
#include "Utils.h"
#include "IRCDDBMultiClient.h"
#include "Log.h"

#define CIRCDDB_VERSION	  "2.0.0"

IRCProtocol::IRCProtocol(IRCApplication *app, const std::string& callsign, const std::string& password, const std::string& channel, const std::string& versionInfo)
{
	m_password = password;
	m_channel = channel;
	m_app = app;

	m_versionInfo.assign("CIRCDDB:");
	m_versionInfo.append(CIRCDDB_VERSION);

	if (versionInfo.size()) {
		m_versionInfo.push_back(' ');
		m_versionInfo.append(versionInfo);
	}

	std::string::size_type hyphenPos = callsign.find('-');

	if (hyphenPos == std::string::npos) {
		m_nicks[0] = callsign + std::string("-1");
		m_nicks[1] = callsign + std::string("-2");
		m_nicks[2] = callsign + std::string("-3");
		m_nicks[3] = callsign + std::string("-4");
	} else {
		m_nicks[0] = callsign;
		m_nicks[1] = callsign;
		m_nicks[2] = callsign;
		m_nicks[3] = callsign;
	}
	m_name = callsign;
	m_pingTimer = 60; // 30 seconds
	m_state = 0;
	m_timer = 0;
	
	chooseNewNick();
}

IRCProtocol::~IRCProtocol()
{
}

void IRCProtocol::chooseNewNick()
{
	int r = rand() % 4;
	m_currentNick = m_nicks[r];
}

void IRCProtocol::setNetworkReady(bool b)
{
	if (b == true) {
		if (0 != m_state)
			CLog::logInfo("IRCProtocol::setNetworkReady: unexpected state\n");
		m_state = 1;
		chooseNewNick();
	} else
		m_state = 0;
}


bool IRCProtocol::processQueues(IRCMessageQueue *recvQ, IRCMessageQueue *sendQ)
{
	if (m_timer > 0)
		m_timer--;

	while (recvQ->messageAvailable()) {
		IRCMessage *m = recvQ->getMessage();
		if (0 == m->command.compare("004")) {
			if (4 == m_state) {
				if (m->params.size() > 1) {
					std::regex serverNamePattern("^grp[1-9]s[1-9].ircDDB$");
					if (std::regex_match(m->params[1], serverNamePattern))
						m_app->setBestServer(std::string("s-") + m->params[1].substr(0,6));
				}
				m_state = 5;  // next: JOIN
				m_app->setCurrentNick(m_currentNick);
			}
		} else if (0 == m->command.compare("PING")) {
			IRCMessage *m2 = new IRCMessage();
			m2->command = std::string("PONG");
			if (m->params.size() > 0) {
				m2->numParams = 1;
				m2->params.push_back(m->params[0]);
			}
			sendQ -> putMessage(m2);
		} else if (0 == m->command.compare("JOIN")) {
			if (m->numParams>=1 && 0==m->params[0].compare(m_channel)) {
				if (0==m->getPrefixNick().compare(m_currentNick) && 6==m_state) {
					if (m_debugChannel.size())
						m_state = 7;  // next: join debug_channel
					else
						m_state = 10; // next: WHO *
				} else if (m_app)
					m_app->userJoin(m->getPrefixNick(), m->getPrefixName(), m->getPrefixHost());
			}

			if (m->numParams>=1 && 0==m->params[0].compare(m_debugChannel)) {
				if (0==m->getPrefixNick().compare(m_currentNick) && 8==m_state)
					m_state = 10; // next: WHO *
			}
		} else if (0 == m->command.compare("PONG")) {
			if (12 == m_state) {
				m_timer = m_pingTimer;
				m_state = 11;
			}
		} else if (0 == m->command.compare("PART")) {
			if (m->numParams>=1 && 0==m->params[0].compare(m_channel)) {
				if (m_app != NULL)
					m_app->userLeave(m->getPrefixNick());
			}
		} else if (0 == m->command.compare("KICK")) {
			if (m->numParams>=2 && 0==m->params[0].compare(m_channel)) {
				if (0 == m->params[1].compare(m_currentNick)) {
					// i was kicked!!
					delete m;
					return false;
				} else if (m_app)
					m_app->userLeave(m->params[1]);
			}
		} else if (0 == m->command.compare("QUIT")) {
			if (m_app)
				m_app->userLeave(m->getPrefixNick());
		} else if (0 == m->command.compare("MODE")) {
			if (m->numParams>=3 && 0==m->params[0].compare(m_channel)) {
				if (m_app) {
					std::string mode = m->params[1];

					for (size_t i=1; i<mode.size() && (size_t)m->numParams>=i+2; i++) {
						if ('o' == mode[i]) {
							if ('+' == mode[0])
								m_app->userChanOp(m->params[i+1], true);
							else if ('-' == mode[0])
								m_app->userChanOp(m->params[i+1], false);
						}
					} // for
				}
			}
		} else if (0 == m->command.compare("PRIVMSG")) {
			if (m->numParams==2 && m_app) {
				if (0 == m->params[0].compare(m_channel) && m_app)
					m_app->msgChannel(m);
				else if (0 == m->params[0].compare(m_currentNick) && m_app)
					m_app->msgQuery(m);
			}
		} else if (0 == m->command.compare("352")) {  // WHO list
			if (m->numParams>=7 && 0==m->params[0].compare(m_currentNick) && 0==m->params[1].compare(m_channel)) {
				if (m_app) {
					m_app->userJoin(m->params[5], m->params[2], m->params[3]);
					m_app->userChanOp(m->params[5], 0==m->params[6].compare("H@"));
				}
			}
		} else if (0 == m->command.compare("433")) { // nick collision
			if (2 == m_state) {
				m_state = 3;  // nick collision, choose new nick
				m_timer = 10; // wait 5 seconds..
			}
		} else if (0==m->command.compare("332") || 0==m->command.compare("TOPIC")) {  // topic
			if (2==m->numParams && m_app && 0==m->params[0].compare(m_channel))
				m_app->setTopic(m->params[1]);
		}

		delete m;
	}

	IRCMessage *m;
	switch (m_state) {
		case 1:
			m = new IRCMessage();
			m->command = std::string("PASS");
			m->numParams = 1;
			m->params.push_back(m_password);
			sendQ->putMessage(m);

			m = new IRCMessage();
			m->command = std::string("NICK");
			m->numParams = 1;
			m->params.push_back(m_currentNick);
			sendQ->putMessage(m);

			m_timer = 10;  // wait for possible nick collision message
			m_state = 2;
			break;

		case 2:
			if (0 == m_timer) {
				m = new IRCMessage();
				m->command = std::string("USER");
				m->numParams = 4;
				m->params.push_back(m_name);
				m->params.push_back(std::string("0"));
				m->params.push_back(std::string("*"));
				m->params.push_back(m_versionInfo);
				sendQ->putMessage(m);

				m_timer = 30;
				m_state = 4; // wait for login message
			}
			break;

		case 3:
			if (0 == m_timer) {
				chooseNewNick();
				m = new IRCMessage();
				m->command = std::string("NICK");
				m->numParams = 1;
				m->params.push_back(m_currentNick);
				sendQ->putMessage(m);

				m_timer = 10;  // wait for possible nick collision message
				m_state = 2;
			}
			break;

		case 4:
			if (0 == m_timer) // no login message received -> disconnect
				return false;
			break;

		case 5:
			m = new IRCMessage();
			m->command = std::string("JOIN");
			m->numParams = 1;
			m->params.push_back(m_channel);
			sendQ->putMessage(m);

			m_timer = 30;
			m_state = 6; // wait for join message
			break;

		case 6:
			if (0 == m_timer) // no join message received -> disconnect
				return false;
			break;

		case 7:
			if (0 == m_debugChannel.size())
				return false; // this state cannot be processed if there is no debug_channel

			m = new IRCMessage();
			m->command = std::string("JOIN");
			m->numParams = 1;
			m->params.push_back(m_debugChannel);
			sendQ->putMessage(m);

			m_timer = 30;
			m_state = 8; // wait for join message
			break;

		case 8:
			if (0 == m_timer) // no join message received -> disconnect
				return false;
			break;

		case 10:
			m = new IRCMessage();
			m->command = std::string("WHO");
			m->numParams = 2;
			m->params.push_back(m_channel);
			m->params.push_back(std::string("*"));
			sendQ->putMessage(m);

			m_timer = m_pingTimer;
			m_state = 11; // wait for timer and then send ping

			if (m_app)
				m_app->setSendQ(sendQ);  // this switches the application on
			break;

		case 11:
			if (0 == m_timer) {
				m = new IRCMessage();
				m->command = std::string("PING");
				m->numParams = 1;
				m->params.push_back(m_currentNick);
				sendQ->putMessage(m);

				m_timer = m_pingTimer;
				m_state = 12; // wait for pong
			}
			break;

		case 12:
			if (0 == m_timer) // no pong message received -> disconnect
				return false;
			break;
	}
	return true;
}


