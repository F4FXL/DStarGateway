/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2012        Jonathan Naylor, G4KLX
Copyright (c) 2017 by Thomas A. Early

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
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>

#include "IRCClient.h"
#include "Utils.h"

IRCClient::IRCClient(IRCApplication *app, const std::string& update_channel, const std::string& hostName, unsigned int port, const std::string& callsign,
										const std::string& password, const std::string& versionInfo, const std::string& localAddr)
{
	CUtils::safeStringCopy(m_host_name, hostName.c_str(), sizeof m_host_name);

	m_callsign = callsign;
	CUtils::ToLower(m_callsign);
	m_port = port;
	m_password = password;
	m_app = app;

	if (0 == localAddr.size())
		CUtils::safeStringCopy(m_local_addr, "0.0.0.0", sizeof m_local_addr);
	else
		CUtils::safeStringCopy(m_local_addr, localAddr.c_str(), sizeof m_local_addr);

	m_proto = new IRCProtocol(app, m_callsign, m_password, update_channel, versionInfo);

	m_recvQ = NULL;
	m_sendQ = NULL;
	m_recv = NULL;
}

IRCClient::~IRCClient()
{
	delete m_proto;
}

void IRCClient::startWork()
{
	m_terminateThread = false;
	m_future = std::async(std::launch::async, &IRCClient::Entry, this);
}

void IRCClient::stopWork()
{
	m_terminateThread = true;
	m_future.get();
}

void IRCClient::Entry()
{
	const unsigned int MAXIPV4ADDR = 10;
	struct sockaddr_in addr[MAXIPV4ADDR];
	struct sockaddr_in myaddr;

	unsigned int numAddr = 0;
	int state = 0;
	int timer = 0;
	int sock = 0;
	unsigned int currentAddr = 0;

	int result = CUtils::getAllIPV4Addresses(m_local_addr, 0, &numAddr, &myaddr, 1);

	if (result || 1!=numAddr) {
		printf("IRCClient::Entry: local address not parseable, using 0.0.0.0\n");
		memset(&myaddr, 0, sizeof(struct sockaddr_in));
	}

	while (true) {
		if (timer > 0)
			timer--;

		switch (state) {
			case 0:
				if (m_terminateThread) {
					printf("IRCClient::Entry: thread terminated at state=%d\n", state);
					return;
				}

				if (timer == 0) {
					timer = 30;

					if (0 == CUtils::getAllIPV4Addresses(m_host_name, m_port, &numAddr, addr, MAXIPV4ADDR)) {
						printf("IRCClient::Entry: number of DNS entries %d\n", numAddr);
						if (numAddr > 0) {
							currentAddr = 0;
							state = 1;
							timer = 0;
						}
					}
				}
				break;

			case 1:
				if (m_terminateThread) {
					printf("IRCClient::Entry: thread terminated at state=%d\n", state);
					return;
				}

				if (timer == 0) {
					sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

					if (sock < 0) {
						printf("IRCClient::Entry: socket\n");
						timer = 30;
						state = 0;
					} else {
						if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
							printf("IRCClient::Entry: fcntl\n");
							close(sock);
							timer = 30;
							state = 0;
						} else {
							unsigned char * h = (unsigned char *) &(myaddr.sin_addr);

							if (h[0] || h[1] || h[2] || h[3])
								printf("IRCClient::Entry: bind: local address %d.%d.%d.%d\n", h[0], h[1], h[2], h[3]);

							int res = bind(sock, (struct sockaddr *)&myaddr, sizeof (struct sockaddr_in));

							if (res) {
								printf("IRCClient::Entry: bind\n");
								close(sock);
								state = 0;
								timer = 30;
								break;
							}

							h = (unsigned char *) &(addr[currentAddr].sin_addr);
							printf("IRCClient::Entry: trying to connect to %d.%d.%d.%d\n", h[0], h[1], h[2], h[3]);

							res = connect(sock, (struct sockaddr *)(addr + currentAddr), sizeof (struct sockaddr_in));

							if (res == 0) {
								printf("IRCClient::Entry: connected\n");
								state = 4;
							} else { 
								if (errno == EINPROGRESS) {
									printf("IRCClient::Entry: connect in progress\n");
									state = 3;
									timer = 10;  // 5 second timeout
								} else {
									printf("IRCClient::Entry: connect\n");
									close(sock);
									currentAddr++;
									if (currentAddr >= numAddr) {
										state = 0;
										timer = 30;
									} else {
										state = 1;
										timer = 4;
									}
								}
							}
						} // connect
					}
				}
				break;

			case 3:
				{
					struct timeval tv;
					tv.tv_sec = 0; 
					tv.tv_usec = 0; 

					fd_set myset;
					FD_ZERO(&myset);
					FD_SET(sock, &myset);

					int res = select(sock+1, NULL, &myset, NULL, &tv); 
					if (res < 0) {
						printf("IRCClient::Entry: select\n");
						close(sock);
						state = 0;
						timer = 30;
					}
					else if (res > 0) // connect is finished
					{
						socklen_t val_len;
						int value;

						val_len = sizeof value;

						if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *) &value, &val_len) < 0) {
							printf("IRCClient::Entry: getsockopt\n");
							close(sock);
							state = 0;
							timer = 30;
						} else {
							if (value) {
								printf("IRCClient::Entry: SO_ERROR=%d\n", value);
								close(sock);
								currentAddr ++;
								if (currentAddr >= numAddr) {
									state = 0;
									timer = 30;
								} else {
									state = 1;
									timer = 2;
								}
							} else {
								printf("IRCClient::Entry: connected2\n");
								state = 4;
							}
						}
					}
					else if (timer == 0) {  // select timeout and timer timeout
						printf("IRCClient::Entry: connect timeout\n");
						close(sock);
						currentAddr ++;
						if (currentAddr >= numAddr) {
							state = 0;
							timer = 30;
						} else {
							state = 1; // open new socket
							timer = 2;
						}
					}
				}
				break;

			case 4:
				{
					m_recvQ = new IRCMessageQueue();
					m_sendQ = new IRCMessageQueue();

					m_recv = new IRCReceiver(sock, m_recvQ);
					m_recv->startWork();

					m_proto->setNetworkReady(true);
					state = 5;
					timer = 0;
				}
				break;


			case 5:
				if (m_terminateThread)
					state = 6;
				else {
					if (m_recvQ->isEOF()) {
						timer = 0;
						state = 6;
					} else if (m_proto->processQueues(m_recvQ, m_sendQ) == false) {
						timer = 0;
						state = 6;
					}
					while (5==state && m_sendQ->messageAvailable()) {
						IRCMessage *m = m_sendQ->getMessage();
						std::string out;

						m->composeMessage(out);

						char buf[200];
						CUtils::safeStringCopy(buf, out.c_str(), sizeof buf);
						int len = strlen(buf);
						if (buf[len - 1] == 10) { // is there a NL char at the end?
							int r = send(sock, buf, len, 0);

							if (r != len) {
								printf("IRCClient::Entry: short write %d < %d\n", r, len);
								timer = 0;
								state = 6;
							}
						} else {
							printf("IRCClient::Entry: no NL at end, len=%d\n", len);
							timer = 0;
							state = 6;
						}
						delete m;
					}
				}
				break;

			case 6:
				{
					if (m_app) {
						m_app->setSendQ(NULL);
						m_app->userListReset();
					}

					m_proto->setNetworkReady(false);
					m_recv->stopWork();

					std::this_thread::sleep_for(std::chrono::seconds(2));

					delete m_recv;
					delete m_recvQ;
					delete m_sendQ;

					close(sock);

					if (m_terminateThread) { // request to end the thread
						printf("IRCClient::Entry: thread terminated at state=%d\n", state);
						return;
					}
					timer = 30;
					state = 0;  // reconnect to IRC server
				}
				break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	return;
}





