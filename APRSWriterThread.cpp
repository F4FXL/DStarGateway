/*
 *   Copyright (C) 2010-2014,2018,2020 by Jonathan Naylor G4KLX
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

#include <cassert>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "APRSWriterThread.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "Defs.h"
#include "Log.h"
#include "Version.h"

// #define	DUMP_TX

const unsigned int APRS_TIMEOUT = 10U;

CAPRSWriterThread::CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port) :
CThread(),
m_username(callsign),
m_password(password),
m_ssid(callsign),
m_socket(hostname, port, address),
m_queue(20U),
m_exit(false),
m_connected(false),
m_reconnectTimer(1000U),
m_tries(0U),
m_APRSReadCallback(NULL),
m_filter(""),
m_clientName(FULL_PRODUCT_NAME)
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!hostname.empty());
	assert(port > 0U);

	m_username[LONG_CALLSIGN_LENGTH - 1U] = ' ';
	boost::trim(m_username);
	boost::to_upper(m_username);

	m_ssid = m_ssid.substr(LONG_CALLSIGN_LENGTH - 1U, 1);
}

CAPRSWriterThread::CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port, const std::string& filter, const std::string& clientName) :
CThread(),
m_username(callsign),
m_password(password),
m_ssid(callsign),
m_socket(hostname, port, address),
m_queue(20U),
m_exit(false),
m_connected(false),
m_reconnectTimer(1000U),
m_tries(0U),
m_APRSReadCallback(NULL),
m_filter(filter),
m_clientName(clientName)
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!hostname.empty());
	assert(port > 0U);

	m_username[LONG_CALLSIGN_LENGTH - 1U] = ' ';
	boost::trim(m_username);
	boost::to_upper(m_username);

	m_ssid = m_ssid.substr(LONG_CALLSIGN_LENGTH - 1U, 1);
}

CAPRSWriterThread::~CAPRSWriterThread()
{
	m_username.clear();
	m_password.clear();
}

bool CAPRSWriterThread::start()
{
	Create();
	Run();

	return true;
}

void* CAPRSWriterThread::Entry()
{
	CLog::logInfo("Starting the APRS Writer thread");

	m_connected = connect();
	if (!m_connected) {
		CLog::logInfo("Connect attempt to the APRS server has failed");
		startReconnectionTimer();
	}

	try {
		while (!m_exit) {
			if (!m_connected) {
				if (m_reconnectTimer.isRunning() && m_reconnectTimer.hasExpired()) {
					m_reconnectTimer.stop();

					m_connected = connect();
					if (!m_connected) {
						CLog::logInfo("Reconnect attempt to the APRS server has failed");
						startReconnectionTimer();
					}
				}
			}

			if (m_connected) {
				m_tries = 0U;

				if(!m_queue.empty()){
					char* p = m_queue.getData();

					std::string text(p);
					CLog::logInfo("APRS ==> %s", text.c_str());

					::strcat(p, "\r\n");

					bool ret = m_socket.write((unsigned char*)p, ::strlen(p));
					if (!ret) {
						m_connected = false;
						m_socket.close();
						CLog::logInfo("Connection to the APRS thread has failed");
						startReconnectionTimer();
					}

					delete[] p;
				}
				{
					std::string line;
					int length = m_socket.readLine(line, APRS_TIMEOUT);

					/*if (length == 0)
						CLog::logWarning(("No response from the APRS server after %u seconds", APRS_TIMEOUT);*/

					if (length < 0) {
						m_connected = false;
						m_socket.close();
						CLog::logError("Error when reading from the APRS server");
						startReconnectionTimer();
					}

					if(length > 0 && line[0] != '#'//check if we have something and if that something is an APRS frame
					    && m_APRSReadCallback != NULL)//do we have someone wanting an APRS Frame?
					{	
						//CLog::logInfo("Received APRS Frame : ") + line);
						m_APRSReadCallback(std::string(line));
					}
				}

			}
		}

		if (m_connected)
			m_socket.close();

		while (!m_queue.empty()) {
			char* p = m_queue.getData();
			delete[] p;
		}
	}
	catch (std::exception& e) {
		std::string message(e.what());
		CLog::logInfo("Exception raised in the APRS Writer thread - \"%s\"", message.c_str());
	}
	catch (...) {
		CLog::logInfo("Unknown exception raised in the APRS Writer thread");
	}

	CLog::logInfo("Stopping the APRS Writer thread");

	return NULL;
}

void CAPRSWriterThread::setReadAPRSCallback(ReadAPRSFrameCallback cb)
{
	m_APRSReadCallback = cb;
}

void CAPRSWriterThread::write(const char* data)
{
	assert(data != NULL);

	if (!m_connected)
		return;

	unsigned int len = ::strlen(data);

	char* p = new char[len + 5U];
	::strcpy(p, data);

	m_queue.addData(p);
}

bool CAPRSWriterThread::isConnected() const
{
	return m_connected;
}

void CAPRSWriterThread::stop()
{
	m_exit = true;

	Wait();
}

void CAPRSWriterThread::clock(unsigned int ms)
{
	m_reconnectTimer.clock(ms);
}

bool CAPRSWriterThread::connect()
{
	bool ret = m_socket.open();
	if (!ret)
		return false;

	//wait for lgin banner
	int length;
	std::string serverResponse("");
	length = m_socket.readLine(serverResponse, APRS_TIMEOUT);
	if (length == 0) {
		CLog::logInfo("No reply from the APRS server after %u seconds", APRS_TIMEOUT);
		m_socket.close();
		return false;
	}
	CLog::logInfo("Received login banner : %s", serverResponse.c_str());

	std::string filter(m_filter);
	if (filter.length() > 0) filter = " filter " + filter;
	std::stringstream connectString;
	connectString 	<< "User " << m_username << "-" << m_ssid
					<< " pass " << m_password
					<< " vers " << (!m_clientName.empty() ? m_clientName : FULL_PRODUCT_NAME)
					<< filter;
	//CLog::logInfo("Connect String : ") + connectString);
	ret = m_socket.writeLine(connectString.str());
	if (!ret) {
		m_socket.close();
		return false;
	}
	
	length = m_socket.readLine(serverResponse, APRS_TIMEOUT);
	if (length == 0) {
		CLog::logInfo("No reply from the APRS server after %u seconds", APRS_TIMEOUT);
		m_socket.close();
		return false;
	}
	if (length < 0) {
		CLog::logInfo("Error when reading from the APRS server");
		m_socket.close();
		return false;
	}

	CLog::logInfo("Response from APRS server: %s", serverResponse.c_str());

	CLog::logInfo("Connected to the APRS server");

	return true;
}

void CAPRSWriterThread::startReconnectionTimer()
{
	// Clamp at a ten minutes reconnect time
	m_tries++;
	if (m_tries > 10U)
		m_tries = 10U;

	m_reconnectTimer.setTimeout(m_tries * 60U);
	m_reconnectTimer.start();
}
