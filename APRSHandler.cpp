/*
 *   Copyright (C) 2010,2011,2012,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cassert>
#include <algorithm>

#include "StringUtils.h"
#include "Log.h"
#include "APRSHandler.h"
#include "DStarDefines.h"
#include "Defs.h"
#include "Log.h"
#include "APRSFrame.h"
#include "APRSParser.h"
#include "APRSFormater.h"
#include "APRSUtils.h"

CAPRSHandler::CAPRSHandler(const std::string& hostname, unsigned int port, const std::string& gateway, const std::string& password, const std::string& address) :
m_thread(NULL),
m_gateway(),
m_address(),
m_port(0U),
m_array(),
m_idFrameProvider(nullptr)
{
	assert(!hostname.empty());
	assert(port > 0U);
	assert(!gateway.empty());
	assert(!password.empty());

	m_thread = new CAPRSHandlerThread(gateway, password, address, hostname, port, "m/20");

	m_gateway = gateway;
	m_gateway = m_gateway.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	boost::trim(m_gateway);
}

CAPRSHandler::~CAPRSHandler()
{
	for(auto it = m_array.begin(); it != m_array.end(); it++) {
		delete it->second;
	}

	m_array.clear();
}

void CAPRSHandler::setPort(const std::string& callsign, const std::string& band, double frequency, double offset, double range, double latitude, double longitude, double agl)
{
	std::string temp = callsign;
	temp.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
	temp += band;

	m_array[temp] = new CAPRSEntry(callsign, band, frequency, offset, range, latitude, longitude, agl);
}

bool CAPRSHandler::open()
{
	return m_thread->start();
}

void CAPRSHandler::writeHeader(const std::string& callsign, const CHeaderData& header)
{
	CAPRSEntry* entry = m_array[callsign];
	if (entry == NULL) {
		CLog::logError("Cannot find the callsign \"%s\" in the APRS array", callsign.c_str());
		return;
	}

	entry->reset();

	CAPRSCollector* collector = entry->getCollector();

	collector->writeHeader(header.getMyCall1());
}

void CAPRSHandler::writeData(const std::string& callsign, const CAMBEData& data)
{
	if (data.isEnd())
		return;

	CAPRSEntry* entry = m_array[callsign];
	if (entry == NULL) {
		CLog::logError("Cannot find the callsign \"%s\" in the APRS array", callsign.c_str());
		return;
	}

	CAPRSCollector* collector = entry->getCollector();

	if (data.isSync()) {
		collector->sync();
		return;
	}

	unsigned char buffer[400U];
	data.getData(buffer, DV_FRAME_MAX_LENGTH_BYTES);

	bool complete = collector->writeData(buffer + VOICE_FRAME_LENGTH_BYTES);
	if (!complete)
		return;

	if (!m_thread->isConnected()) {
		collector->reset();
		return;
	}

	// Check the transmission timer
	bool ok = entry->isOK();
	if (!ok) {
		collector->reset();
		return;
	}

	unsigned int length = collector->getData(SLOW_DATA_TYPE_GPS, buffer, 400U);

	if(length == 0U) {
		return;
	}
	
	std::string text((char*)buffer, length);

	CAPRSFrame frame;
	if(!CAPRSParser::parseFrame(text, frame)) {
		collector->reset();
		CLog::logWarning("Failed to parse DPRS Frame : %s", text.c_str());
		return;
	}

	// If we already have a q-construct, don't send it on
	if(std::any_of(frame.getPath().begin(), frame.getPath().end(), [] (std::string s) { return !s.empty() && s[0] == 'q'; })) {
		CLog::logWarning("DPRS Frame already has q construct, not forwarding to APRS-IS: %s", text.c_str());
		return;
	}

	frame.getPath().push_back("qAR");
	frame.getPath().push_back(CStringUtils::string_format("%s-%s", entry->getCallsign().c_str(), entry->getBand().c_str()));
	
	std::string output ;
	CAPRSFormater::frameToString(output, frame);

	m_thread->write(frame);

	collector->reset();
}

void CAPRSHandler::writeStatus(const std::string& callsign, const std::string status)
{
	CAPRSEntry* entry = m_array[callsign];
	if (entry == NULL) {
		CLog::logError("Cannot find the callsign \"%s\" in the APRS array", callsign.c_str());
		return;
	}

	entry->getStatus().setStatus(status);
}

void CAPRSHandler::clock(unsigned int ms)
{
	m_thread->clock(ms);

	if(m_idFrameProvider != nullptr) {
		m_idFrameProvider->clock(ms);
		if(m_idFrameProvider->wantsToSend()) {
			sendIdFrames();
		}
	}

	for (auto it : m_array) {
		if(it.second != NULL) {
			it.second->clock(ms);
			if(it.second->getStatus().isOutOfDate())
				sendStatusFrame(it.second);
		}
	}
}

void CAPRSHandler::sendStatusFrame(CAPRSEntry * entry)
{
	assert(entry != nullptr);

	if(!m_thread->isConnected())
		return;


	auto linkStatus = entry->getStatus();
	std::string body = boost::trim_copy(linkStatus.getStatus());

	if(body[0] != '>')
		body.insert(0, ">");

	std::string sourCall = entry->getCallsign() + '-' + entry->getBand();
	
	CAPRSFrame frame(sourCall,
					 "APD5T3",
					 { "TCPIP*", "qAC", sourCall + "S" },
					 body,
					 APFT_STATUS);

	m_thread->write(frame);

}

void CAPRSHandler::sendIdFrames()
{
	if(m_thread->isConnected())
	{
		for(auto entry : m_array) {
			std::vector<CAPRSFrame *> frames;
			if(m_idFrameProvider->buildAPRSFrames(m_gateway, entry.second, frames)) {
				for(auto frame : frames) {
					m_thread->write(*frame);
					delete frame;
				}
			}
		}
	}
}

bool CAPRSHandler::isConnected() const
{
	return m_thread->isConnected();
}

void CAPRSHandler::close()
{
	if(m_idFrameProvider != nullptr) {
		m_idFrameProvider->close();
		delete m_idFrameProvider;
		m_idFrameProvider = nullptr;
	}

	m_thread->stop();
}

void CAPRSHandler::addReadAPRSCallback(IReadAPRSFrameCallback* cb)
{
	m_thread->addReadAPRSCallback(cb);
}
