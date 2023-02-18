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
#include "IAPRSHandlerBackend.h"
#include "APRSFrame.h"


class CAPRSISHandlerThread : public CThread, IAPRSHandlerBackend {
public:
	CAPRSISHandlerThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port);
	CAPRSISHandlerThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port, const std::string& filter);
	virtual ~CAPRSISHandlerThread();

	bool start();

	bool isConnected() const;

	void write(CAPRSFrame& data);

	void* Entry();

	void stop();

	void clock(unsigned int ms);

	void addReadAPRSCallback(IReadAPRSFrameCallback* cb);

private:
	std::string               m_username;
	std::string               m_password;
	std::string	           m_ssid;
	CTCPReaderWriterClient m_socket;
	CRingBuffer<std::string>     m_queue;
	bool                   m_exit;
	bool                   m_connected;
	CTimer                 m_reconnectTimer;
	CTimer				   m_keepAliveTimer;
	unsigned int           m_tries;
	std::vector<IReadAPRSFrameCallback *>  m_APRSReadCallbacks;
	std::string               m_filter;
	std::string               m_clientName;

	bool connect();
	void startReconnectionTimer();
};

#endif
