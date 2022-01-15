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

#include "G2ProtocolHandler.h"
#include "Utils.h"
#include "Log.h"

// #define	DUMP_TX

const unsigned int BUFFER_LENGTH = 255U;

CG2ProtocolHandler::CG2ProtocolHandler(unsigned int port, const std::string& addr) :
m_socket(addr, port),
m_type(GT_NONE),
m_buffer(NULL),
m_length(0U),
m_address(),
m_port(0U)
{
	m_buffer = new unsigned char[BUFFER_LENGTH];
}

CG2ProtocolHandler::~CG2ProtocolHandler()
{
	delete[] m_buffer;
	m_portmap.clear();
}

bool CG2ProtocolHandler::open()
{
	return m_socket.open();
}

bool CG2ProtocolHandler::writeHeader(const CHeaderData& header)
{
	unsigned char buffer[60U];
	unsigned int length = header.getG2Data(buffer, 60U, true);

#if defined(DUMP_TX)
	CUtils::dump("Sending Header", buffer, length);
#endif

	in_addr addr = header.getYourAddress();
	auto found = m_portmap.find(addr.s_addr);
	unsigned int port = (m_portmap.end()==found) ? header.getYourPort() : found->second;

	for (unsigned int i = 0U; i < 5U; i++) {
		bool res = m_socket.write(buffer, length, addr, port);
		if (!res)
			return false;
	}

	return true;
}

bool CG2ProtocolHandler::writeAMBE(const CAMBEData& data)
{
	unsigned char buffer[40U];
	unsigned int length = data.getG2Data(buffer, 40U);

#if defined(DUMP_TX)
	CUtils::dump("Sending Data", buffer, length);
#endif

	in_addr addr = data.getYourAddress();
	auto found = m_portmap.find(addr.s_addr);
	unsigned int port = (m_portmap.end()==found) ? data.getYourPort() : found->second;

	return m_socket.write(buffer, length, addr, port);
}

G2_TYPE CG2ProtocolHandler::read()
{
	bool res = true;

	// Loop until we have no more data from the socket or we have data for the higher layers
	while (res)
		res = readPackets();

	return m_type;
}

bool CG2ProtocolHandler::readPackets()
{
	m_type = GT_NONE;

	// No more data?
	int length = m_socket.read(m_buffer, BUFFER_LENGTH, m_address, m_port);
	if (length <= 0)
		return false;

	m_length = length;

	// save the incoming port (this is to enable mobile hotspots)
	if (m_portmap.end() == m_portmap.find(m_address.s_addr)) {
		CLog::logInfo("new address %s on port %u\n", inet_ntoa(m_address), m_port);
		m_portmap[m_address.s_addr] = m_port;
	} else {
		if (m_portmap[m_address.s_addr] != m_port) {
			CLog::logInfo("new port for %s is %u, was %u\n", inet_ntoa(m_address), m_port, m_portmap[m_address.s_addr]);
			m_portmap[m_address.s_addr] = m_port;
		}
	}

	if (m_buffer[0] != 'D' || m_buffer[1] != 'S' || m_buffer[2] != 'V' || m_buffer[3] != 'T') {
		return true;
	} else {
		// Header or data packet type?
		if ((m_buffer[14] & 0x80) == 0x80)
			m_type = GT_HEADER;
		else
			m_type = GT_AMBE;

		return false;
	}
}

CHeaderData* CG2ProtocolHandler::readHeader()
{
	if (m_type != GT_HEADER)
		return NULL;

	CHeaderData* header = new CHeaderData;

	// G2 checksums are unreliable
	bool res = header->setG2Data(m_buffer, m_length, false, m_address, m_port);
	if (!res) {
		delete header;
		return NULL;
	}

	return header;
}

CAMBEData* CG2ProtocolHandler::readAMBE()
{
	if (m_type != GT_AMBE)
		return NULL;

	CAMBEData* data = new CAMBEData;

	bool res = data->setG2Data(m_buffer, m_length, m_address, m_port);
	if (!res) {
		delete data;
		return NULL;
	}

	return data;
}

void CG2ProtocolHandler::close()
{
	m_socket.close();
}

void CG2ProtocolHandler::traverseNat(const std::string& address)
{
	unsigned char buffer[1];
	::memset(buffer, 0, 1);
	
	in_addr addr = CUDPReaderWriter::lookup(address);

	//wxLogError(wxT("Punching hole to %s"), address.mb_str());

	m_socket.write(buffer, 1, addr, G2_DV_PORT);
}