/*
 *   Copyright (C) 2010,2011,2012,2018,2020 by Jonathan Naylor G4KLX
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

#ifndef	APRSWriterThread_H
#define	APRSWriterThread_H

#include <vector>

#include "TCPReaderWriterClient.h"
#include "RingBuffer.h"
#include "Timer.h"
#include "Thread.h"
#include "ReadAPRSFrameCallback.h"
#include "APRSFrame.h"


class CAPRSWriterThread : public CThread {
public:
	CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port);
	CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port, const std::string& filter, const std::string& clientName);
	virtual ~CAPRSWriterThread();

	bool start();

	bool isConnected() const;

	void write(CAPRSFrame& data);

	void* Entry();

	void stop();

	void clock(unsigned int ms);

	void addReadAPRSCallback(CReadAPRSFrameCallback* cb);

private:
	std::string               m_username;
	std::string               m_password;
	std::string	           m_ssid;
	CTCPReaderWriterClient m_socket;
	CRingBuffer<std::string>     m_queue;
	bool                   m_exit;
	bool                   m_connected;
	CTimer                 m_reconnectTimer;
	unsigned int           m_tries;
	std::vector<CReadAPRSFrameCallback *>  m_APRSReadCallbacks;
	std::string               m_filter;
	std::string               m_clientName;

	bool connect();
	void startReconnectionTimer();
};

#endif
