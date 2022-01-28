/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early N7TAE
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

#include <unordered_map>
#include <sys/socket.h>

#include "UDPReaderWriter.h"
#include "DStarDefines.h"
#include "HeaderData.h"
#include "AMBEData.h"
#include "NetUtils.h"
#include "Timer.h"

enum G2_TYPE {
	GT_NONE,
	GT_HEADER,
	GT_AMBE
};

class CG2ProtocolHandler {
public:
	CG2ProtocolHandler(CUDPReaderWriter* socket, const struct sockaddr_storage& destination, unsigned int bufferSize);
	~CG2ProtocolHandler();

	bool open();

	bool writeHeader(const CHeaderData& header);
	bool writeAMBE(const CAMBEData& data);

	CHeaderData* readHeader();
	CAMBEData*   readAMBE();

	struct sockaddr_storage getDestination() { return m_address; }
	G2_TYPE getType() { return m_type; }

	bool setBuffer(unsigned char * buffer, int length);

	void clock(unsigned int ms) { m_inactivityTimer.clock(ms); }
	bool isInactive() { return m_inactivityTimer.hasExpired(); }

private:
	CUDPReaderWriter * m_socket;
	G2_TYPE          m_type;
	unsigned char*   m_buffer;
	unsigned int     m_length;
	struct sockaddr_storage m_address;
	CTimer m_inactivityTimer;
	unsigned int m_id;

	bool readPackets();
};
