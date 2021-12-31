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

#include "StringUtils.h"
#include "Log.h"
#include "APRSWriter.h"
#include "DStarDefines.h"
#include "Defs.h"
#include "Log.h"

CAPRSWriter::CAPRSWriter(const std::string& hostname, unsigned int port, const std::string& gateway, const std::string& password, const std::string& address) :
m_thread(NULL),
m_gateway(),
m_address(),
m_port(0U),
m_array()
{
	assert(!hostname.empty());
	assert(port > 0U);
	assert(!gateway.empty());
	assert(!password.empty());

	m_thread = new CAPRSWriterThread(gateway, password, address, hostname, port);

	m_gateway = gateway;
	m_gateway = m_gateway.substr(0, LONG_CALLSIGN_LENGTH - 1U);
	boost::trim(m_gateway);
}

CAPRSWriter::~CAPRSWriter()
{
	for(auto it = m_array.begin(); it != m_array.end(); it++) {
		delete it->second;
	}

	m_array.clear();
}

void CAPRSWriter::setPort(const std::string& callsign, const std::string& band, double frequency, double offset, double range, double latitude, double longitude, double agl)
{
	std::string temp = callsign;
	temp.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
	temp += band;

	m_array[temp] = new CAPRSEntry(callsign, band, frequency, offset, range, latitude, longitude, agl);
}

bool CAPRSWriter::open()
{
	return m_thread->start();
}

void CAPRSWriter::writeHeader(const std::string& callsign, const CHeaderData& header)
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

void CAPRSWriter::writeData(const std::string& callsign, const CAMBEData& data)
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

	unsigned int length = collector->getData(buffer, 400U);
	std::string text((char*)buffer, length);

	auto n = text.find(':');
	if (n == std::string::npos) {
		collector->reset();
		return;
	}

	std::string header = text.substr(0, n);
	std::string body   = text.substr(n + 1U);

	// If we already have a q-construct, don't send it on
	n = header.find('q');
	if (n != std::string::npos)
		return;

	// Remove the trailing \r
	n = body.find('\r');
	if (n != std::string::npos)
		body = body.substr(0, n);

	std::string output = CStringUtils::string_format("%s,qAR,%s-%s:%s", header.c_str(), entry->getCallsign().c_str(), entry->getBand().c_str(), body.c_str());

	char ascii[500U];
	::memset(ascii, 0x00, 500U);
	for (unsigned int i = 0U; i < output.length(); i++)
		ascii[i] = output[i];

	m_thread->write(ascii);

	collector->reset();
}

void CAPRSWriter::clock(unsigned int ms)
{
	m_thread->clock(ms);

	if(m_idFrameProvider != nullptr) {
		m_idFrameProvider->clock(ms);

		if(m_idFrameProvider->wantsToSend() && m_thread->isConnected()) {
			for(auto entry : m_array) {
				std::vector<std::string> frames;
				if(m_idFrameProvider->buildAPRSFrames(m_gateway, entry.second, frames)) {
					for(auto frame : frames) {
						m_thread->write(frame.c_str());
					}
				}
			}
		}
	}

	for (auto it = m_array.begin(); it != m_array.end(); ++it) {
		if(it->second != NULL)
			it->second->clock(ms);
	}
}

bool CAPRSWriter::isConnected() const
{
	return m_thread->isConnected();
}

void CAPRSWriter::close()
{
	if(m_idFrameProvider != nullptr) {
		m_idFrameProvider->close();
		delete m_idFrameProvider;
		m_idFrameProvider = nullptr;
	}

	m_thread->stop();
}
