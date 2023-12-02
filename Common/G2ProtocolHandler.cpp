/*
 *   Copyright (C) 2010,2011,2013 by Jonathan Naylor G4KLX
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

#include <string>
#include <cstring>

#include "G2ProtocolHandler.h"
#include "Utils.h"
#include "Log.h"

// #define	DUMP_TX

const unsigned int BUFFER_LENGTH = 255U;

CG2ProtocolHandler::CG2ProtocolHandler(CUDPReaderWriter* socket, const struct sockaddr_storage& destination, unsigned int bufferSize) :
m_socket(socket),
m_type(GT_NONE),
m_buffer(nullptr),
m_length(0U),
m_address(destination),
m_inactivityTimer(1000U, 29U),
m_id(0U)
{
	m_inactivityTimer.start();
	m_buffer = new unsigned char[bufferSize];
	::memset(m_buffer, 0, bufferSize);
}

CG2ProtocolHandler::~CG2ProtocolHandler()
{
	delete[] m_buffer;
}

bool CG2ProtocolHandler::writeHeader(const CHeaderData& header)
{
	m_inactivityTimer.start();
	unsigned char buffer[60U];
	unsigned int length = header.getG2Data(buffer, 60U, true);

#if defined(DUMP_TX)
	CUtils::dump("Sending Header", buffer, length);
#endif

	assert(CNetUtils::match(header.getDestination(), m_address, IMT_ADDRESS_ONLY));

	//CLog::logTrace("Write header to %s:%u", inet_ntoa(addr), ntohs(TOIPV4(m_address)->sin_port));

	for (unsigned int i = 0U; i < 5U; i++) {
		bool res = m_socket->write(buffer, length, m_address);
		if (!res)
			return false;
	}

	return true;
}

bool CG2ProtocolHandler::writeAMBE(const CAMBEData& data)
{
	m_inactivityTimer.start();
	unsigned char buffer[40U];
	unsigned int length = data.getG2Data(buffer, 40U);

#if defined(DUMP_TX)
	CUtils::dump("Sending Data", buffer, length);
#endif

	assert(CNetUtils::match(data.getYourAddressAndPort(), m_address, IMT_ADDRESS_ONLY));
	//CLog::logTrace("Write ambe to %s:%u", inet_ntoa(addr), ntohs(TOIPV4(m_address)->sin_port));
	return m_socket->write(buffer, length, m_address);
}

bool CG2ProtocolHandler::setBuffer(unsigned char * buffer, int length)
{
	assert(buffer != nullptr);

	m_type = GT_NONE;
	::memcpy(m_buffer, buffer, length);

	if(length <= 0)
		return false;

	m_length = length;

	if (m_buffer[0] != 'D' || m_buffer[1] != 'S' || m_buffer[2] != 'V' || m_buffer[3] != 'T') {
		CLog::logTrace("DSVT");
		return true;
	} else {
		// Header or data packet type?
		if ((m_buffer[14] & 0x80) == 0x80) {
			m_type = GT_HEADER;
		}
		else {
			m_type = GT_AMBE;
		}

		return false;
	}
}

CHeaderData* CG2ProtocolHandler::readHeader()
{
	m_inactivityTimer.start();
	if (m_type != GT_HEADER || m_id != 0U)
		return nullptr;

	m_type = GT_NONE; // Header data has been consumed, reset our status
	CHeaderData* header = new CHeaderData;

	// G2 checksums are unreliable
	bool res = header->setG2Data(m_buffer, m_length, false, TOIPV4(m_address)->sin_addr,  ntohs(GETPORT(m_address)));
	if (!res) {
		delete header;
		return nullptr;
	}

	m_id = header->getId();// remember the id so we do not read it duplicate

	return header;
}

CAMBEData* CG2ProtocolHandler::readAMBE()
{
	m_inactivityTimer.start();
	if (m_type != GT_AMBE)
		return NULL;

	m_type = GT_NONE; // Ambe data has been consumed, reset our status
	CAMBEData* data = new CAMBEData;

	bool res = data->setG2Data(m_buffer, m_length, TOIPV4(m_address)->sin_addr, ntohs(GETPORT(m_address)));
	if (!res) {
		delete data;
		return NULL;
	}

	if(data->isEnd())
		m_id = 0U;

	return data;
}

