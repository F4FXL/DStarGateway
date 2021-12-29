/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)
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

#include <sys/types.h>
#include <sys/socket.h>

#include "IRCReceiver.h"
#include "Utils.h"
#include "Log.h"

IRCReceiver::IRCReceiver(int sock, IRCMessageQueue *q)
{
	m_sock = sock;
	m_recvQ = q;
}

IRCReceiver::~IRCReceiver()
{
}

void IRCReceiver::startWork()
{
	m_terminateThread = false;
	m_future = std::async(std::launch::async, &IRCReceiver::Entry, this);
}

void IRCReceiver::stopWork()
{
	m_terminateThread = true;
	m_future.get();
}

static int doRead(int sock, char *buf, int buf_size)
{
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	fd_set rdset;
	fd_set errset;

	FD_ZERO(&rdset);
	FD_ZERO(&errset);
	FD_SET(sock, &rdset);
	FD_SET(sock, &errset);

	int res = select(sock+1, &rdset, NULL, &errset, &tv);

	if (res < 0) {
		CLog::logInfo("IRCReceiver::doRead: select\n");
		return -1;
	} else if (res > 0) {
		if (FD_ISSET(sock, &errset)) {
			CLog::logInfo("IRCReceiver::doRead: select (FD_ISSET(sock, exceptfds))\n");
			return -1;
		}

		if (FD_ISSET(sock, &rdset)) {
			res = recv(sock, buf, buf_size, 0);
			if (res < 0) {
				CLog::logInfo("IRCReceiver::doRead: read\n");
				return -1;
			} else if (res == 0) {
				CLog::logInfo("IRCReceiver::doRead: EOF read==0\n");
				return -1;
			} else
				return res;
		}
	}
	return 0;
}

void IRCReceiver::Entry()
{
	IRCMessage *m = new IRCMessage();
	int state = 0;

	while (! m_terminateThread) {
		char buf[200];
		int r = doRead(m_sock, buf, sizeof buf);

		if (r < 0) {
			m_recvQ->signalEOF();
			delete m;  // delete unfinished IRCMessage
			break;
		}

		for (int i=0; i < r; i++) {
			char b = buf[i];
			if (b > 0) {
				if (b == '\n') {
					m_recvQ->putMessage(m);
					m = new IRCMessage();
					state = 0;
				}
				else if (b != '\r') {
					switch (state) {
						case 0:	// command
							if (b == ':')
								state = 1; // prefix
							else if (b != ' ') {
								m->command.push_back(b);
								state = 2; // command
							}
							break;

						case 1:	// prefix
							if (b == ' ')
								state = 2; // command is next
							else
								m->prefix.push_back(b);
							break;

						case 2:
							if (b == ' ') {
								state = 3; // params are next
								m->numParams = 1;
								m->params.push_back(std::string(""));
							} else
								m->command.push_back(b);
							break;

						case 3:
							if (b == ' ') {
								m->numParams++;
								if (m->numParams >= 15)
									state = 5; // ignore the rest
								m->params.push_back(std::string(""));
							} else if (b==':' && m->params[m->numParams-1].size()==0)
								state = 4; // rest of line is this param
							else
								m->params[m->numParams-1].push_back(b);
							break;

						case 4:
							m->params[m->numParams-1].push_back(b);
							break;
					} // switch
				}
			} // if
		} // for
	} // while
	return;
}

