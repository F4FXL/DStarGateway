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

#include "APRSHandlerThread.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "Defs.h"
#include "Log.h"
#include "Version.h"
#include "APRSFormater.h"
#include "APRSParser.h"

// #define	DUMP_TX

const unsigned int APRS_TIMEOUT = 10U;
const unsigned int APRS_READ_TIMEOUT = 1U;
const unsigned int APRS_KEEP_ALIVE_TIMEOUT = 60U;

CAPRSHandlerThread::CAPRSHandlerThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port) :
CThread(),
m_username(callsign),
m_password(password),
m_ssid(callsign),
m_socket(hostname, port, address),
m_queue(20U),
m_exit(false),
m_connected(false),
m_reconnectTimer(1000U),
m_keepAliveTimer(1000U, APRS_KEEP_ALIVE_TIMEOUT),
m_tries(0U),
m_APRSReadCallbacks(),
m_filter(),
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

CAPRSHandlerThread::CAPRSHandlerThread(const std::string& callsign, const std::string& password, const std::string& address, const std::string& hostname, unsigned int port, const std::string& filter) :
CThread(),
m_username(callsign),
m_password(password),
m_ssid(callsign),
m_socket(hostname, port, address),
m_queue(20U),
m_exit(false),
m_connected(false),
m_reconnectTimer(1000U),
m_keepAliveTimer(1000U, APRS_KEEP_ALIVE_TIMEOUT),
m_tries(0U),
m_APRSReadCallbacks(),
m_filter(filter),
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

CAPRSHandlerThread::~CAPRSHandlerThread()
{
	std::vector<IReadAPRSFrameCallback *> callBacksCopy;
	callBacksCopy.assign(m_APRSReadCallbacks.begin(), m_APRSReadCallbacks.end());

	m_APRSReadCallbacks.clear();

	callBacksCopy.clear();

	m_username.clear();
	m_password.clear();
}

bool CAPRSHandlerThread::start()
{
	Create();
	Run();

	return true;
}

void* CAPRSHandlerThread::Entry()
{
	CLog::logInfo("Starting the APRS Writer thread");

	m_connected = connect();
	if (!m_connected) {
		CLog::logInfo("Connect attempt to the APRS server has failed");
		startReconnectionTimer();
	}

	try {
		m_keepAliveTimer.start();
		while (!m_exit) {
			if (!m_connected) {
				Sleep(100U);
				if (m_reconnectTimer.isRunning() && m_reconnectTimer.hasExpired()) {
					m_reconnectTimer.stop();

					CLog::logDebug("Trying to reconnect to the APRS server");
					m_connected = connect();
					if (!m_connected) {
						CLog::logInfo("Reconnect attempt to the APRS server has failed");
						startReconnectionTimer();
					}
					else {
						m_keepAliveTimer.start();
					}
				}
			}

			if (m_connected) {
				m_tries = 0U;

				if(!m_queue.empty()){
					auto frameStr = m_queue.getData();

					CLog::logInfo("APRS ==> %s", frameStr.c_str());

					bool ret = m_socket.writeLine(frameStr);
					if (!ret) {
						m_connected = false;
						m_socket.close();
						CLog::logInfo("Error when writing to the APRS server");
						startReconnectionTimer();
					}
				}
				{
					std::string line;
					int length = m_socket.readLine(line, APRS_READ_TIMEOUT);

					if (length < 0 || m_keepAliveTimer.hasExpired()) {
						m_connected = false;
						m_socket.close();
						CLog::logError("Error when reading from the APRS server");
						startReconnectionTimer();
					}
					else if(length > 0 && line[0] == '#') {
						m_keepAliveTimer.start();
					}
					else if(line.length() > 0 && line[0] != '#') {
						m_keepAliveTimer.start();
						CLog::logDebug("APRS <== %s", line.c_str());
						CAPRSFrame readFrame;
						if(CAPRSParser::parseFrame(line, readFrame)) {
							for(auto cb : m_APRSReadCallbacks) {
								CAPRSFrame f(readFrame);
								cb->readAPRSFrame(f);
							}
						}
					}
				}
			}
		}

		if (m_connected)
			m_socket.close();

		while (!m_queue.empty()) {
			auto s = m_queue.getData();
			s.clear();
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

void CAPRSHandlerThread::addReadAPRSCallback(IReadAPRSFrameCallback * cb)
{
	assert(cb != nullptr);
	m_APRSReadCallbacks.push_back(cb);
}

void CAPRSHandlerThread::write(CAPRSFrame& frame)
{
	if (!m_connected)
		return;

	std::string frameString;
	if(CAPRSFormater::frameToString(frameString, frame)) {
		boost::trim_if(frameString, [] (char c) { return c == '\r' || c == '\n'; }); // trim all CRLF, we will add our own, just to make sure we get rid of any garbage that might come from slow data
		CLog::logTrace("Queued APRS Frame : %s", frameString.c_str());
		frameString.append("\r\n");

		m_queue.addData(frameString);
	}
}

bool CAPRSHandlerThread::isConnected() const
{
	return m_connected;
}

void CAPRSHandlerThread::stop()
{
	m_exit = true;

	Wait();
}

void CAPRSHandlerThread::clock(unsigned int ms)
{
	m_reconnectTimer.clock(ms);
	m_keepAliveTimer.clock(ms);
}

bool CAPRSHandlerThread::connect()
{
	m_socket.close();
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
		CLog::logInfo("Error when reading from the APRS server (connect)");
		m_socket.close();
		return false;
	}

	CLog::logInfo("Response from APRS server: %s", serverResponse.c_str());

	CLog::logInfo("Connected to the APRS server");

	return true;
}

void CAPRSHandlerThread::startReconnectionTimer()
{
	// Clamp at a ten minutes reconnect time
	m_tries++;
	if (m_tries > 10U)
		m_tries = 10U;

	CLog::logDebug("Next APRS reconnection try in %u minute", m_tries);

	m_reconnectTimer.setTimeout(m_tries * 60U);
	m_reconnectTimer.start();
}
