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

#pragma once

#include "Defs.h"

#include <unordered_map>
#include <string>


#include "APRSEntry.h"
#include "APRSWriterThread.h"
#include "UDPReaderWriter.h"
#include "APRSCollector.h"
#include "DStarDefines.h"
#include "HeaderData.h"
#include "AMBEData.h"
#include "Timer.h"
#include "APRSIdFrameProvider.h"

class CAPRSHandler {
public:
	CAPRSHandler(const std::string& hostname, unsigned int port, const std::string& gateway, const std::string& password, const std::string& address);
	~CAPRSHandler();

	bool open();

	void setIdFrameProvider(CAPRSIdFrameProvider * idFrameProvider) { m_idFrameProvider = idFrameProvider; }

	void setPort(const std::string& callsign, const std::string& band, double frequency, double offset, double range, double latitude, double longitude, double agl);

	void writeHeader(const std::string& callsign, const CHeaderData& header);

	void writeData(const std::string& callsign, const CAMBEData& data);

	void  writeStatus(const std::string& callsign, const std::string status);

	bool isConnected() const;

	void clock(unsigned int ms);

	void close();

private:
	CAPRSWriterThread*		m_thread;
	std::string				m_gateway;
	in_addr					m_address;
	unsigned int			m_port;
	std::unordered_map<std::string,CAPRSEntry *>	m_array;
	CAPRSIdFrameProvider * m_idFrameProvider;

	void sendIdFrames();
	void sendStatusFrame(CAPRSEntry * entrry);
};

