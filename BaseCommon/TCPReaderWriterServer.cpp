/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
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

#include "TCPReaderWriterServer.h"
#include "Log.h"
#include "NetUtils.h"

#include <cerrno>
#include <cassert>


CTCPReaderWriterServer::CTCPReaderWriterServer(const std::string& address, unsigned int port) :
CThread("TCP server"),
m_address(address),
m_port(port),
m_fd(-1),
m_client(NULL),
m_stopped(false)
{
	assert(port > 0U);
}

CTCPReaderWriterServer::~CTCPReaderWriterServer()
{
}

bool CTCPReaderWriterServer::start()
{
	bool ret = open();
	if (!ret) {
		close();
		return false;
	}

	Create();
	Run();

	return true;
}

int CTCPReaderWriterServer::read(unsigned char* buffer, unsigned int length, unsigned int secs)
{
	assert(buffer != NULL);
	assert(length > 0U);

	if (m_client != NULL) {
		int ret = m_client->read(buffer, length, secs);
		if (ret < 0) {
			CLog::logInfo("Lost TCP connection to port %u", m_port);

			m_client->close();
			delete m_client;
			m_client = NULL;

			open();

			return 0;
		}

		return ret;
	}

	return 0;
}

bool CTCPReaderWriterServer::write(const unsigned char* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(length > 0U);

	if (m_client != NULL) {
		bool ret = m_client->write(buffer, length);
		if (!ret) {
			CLog::logInfo("Lost TCP connection to port %u", m_port);

			m_client->close();
			delete m_client;
			m_client = NULL;

			open();

			return false;
		}

		return true;
	}

	return true;
}

void* CTCPReaderWriterServer::Entry()
{
#ifndef DEBUG_DSTARGW
	try {
#endif
		while (!m_stopped) {
			int ret = accept();
			switch (ret) {
				case -2:
					break;
				case -1:
					break;
				default:
					CLog::logInfo("Incoming TCP connection to port %u", m_port);
					m_client = new CTCPReaderWriterClient(ret);
					close();
					break;
			}

			Sleep(1000UL);
		}

		if (m_client != NULL) {
			m_client->close();
			delete m_client;
		}

		close();

#ifndef DEBUG_DSTARGW
	}
	catch (std::exception& e) {
		std::string message(e.what());
		CLog::logError("Exception raised in the TCP Reader-Writer Server thread - \"%s\"", message.c_str());
	}
	catch (...) {
		CLog::logError("Unknown exception raised in the TCP Reader-Writer Server thread");
	}
#endif

	return NULL;
}

void CTCPReaderWriterServer::stop()
{
	m_stopped = true;

	Wait();
}

bool CTCPReaderWriterServer::open()
{
	m_fd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_fd < 0) {
		CLog::logError("Cannot create the TCP server socket, err=%d", errno);
		return false;
	}

	struct sockaddr_in addr;
	::memset(&addr, 0x00, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(m_port);
	if (m_address.empty())
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr = lookup(m_address);

	if (addr.sin_addr.s_addr == INADDR_NONE) {
		CLog::logError("The address is invalid - %s", m_address.c_str());
		close();
		return false;
	}

	int reuse = 1;
	if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == -1) {
		CLog::logError("Cannot set the TCP server socket option, err=%d", errno);
		close();
		return false;
	}

	if (::bind(m_fd, (sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
		CLog::logError("Cannot bind the TCP server address, err=%d", errno);
		close();
		return false;
	}

	::listen(m_fd, 5);

	return true;
}

int CTCPReaderWriterServer::accept()
{
	if (m_fd == -1)
		return -1;

	// Check that the accept() won't block
	fd_set readFds;
	FD_ZERO(&readFds);
#if defined(__WINDOWS__)
	FD_SET((unsigned int)m_fd, &readFds);
#else
	FD_SET(m_fd, &readFds);
#endif

	// Return after timeout
	timeval tv;
	tv.tv_sec  = 0L;
	tv.tv_usec = 0L;

	int ret = ::select(m_fd + 1, &readFds, NULL, NULL, &tv);
	if (ret < 0) {
		CLog::logError("Error returned from TCP server select, err=%d", errno);
		return -2;
	}

#if defined(__WINDOWS__)
	if (!FD_ISSET((unsigned int)m_fd, &readFds))
		return -1;
#else
	if (!FD_ISSET(m_fd, &readFds))
		return -1;
#endif

	struct sockaddr_in addr;
#if defined(__WINDOWS__)
	int len = sizeof(struct sockaddr_in);
#else
	socklen_t len = sizeof(struct sockaddr_in);
#endif

	ret = ::accept(m_fd, (sockaddr*)&addr, &len);
	if (ret < 0) {
		CLog::logError("Error returned from TCP server accept, err=%d", errno);
	}

	return ret;
}

void CTCPReaderWriterServer::close()
{
	if (m_fd != -1) {
		::close(m_fd);
		m_fd = -1;
	}
}

in_addr CTCPReaderWriterServer::lookup(const std::string& hostname) const
{
	in_addr addrv4;
	addrv4.s_addr = INADDR_NONE;

	sockaddr_storage addr;
	auto res = CNetUtils::lookupV4(hostname, addr);
	
	if(res) {
		addrv4 = TOIPV4(addr)->sin_addr;
	}

	return addrv4;
}
