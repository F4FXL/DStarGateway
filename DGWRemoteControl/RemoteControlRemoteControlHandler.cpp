/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#include <string.h>
#include <cassert>

#include "Utils.h"
#include "RemoteControlRemoteControlHandler.h"
#include "DStarDefines.h"

const unsigned int BUFFER_LENGTH = 2000U;

const unsigned int MAX_RETRIES = 3U;

CRemoteControlRemoteControlHandler::CRemoteControlRemoteControlHandler(const std::string& address, unsigned int port) :
m_socket("", 0U),
m_address(),
m_port(port),
m_loggedIn(false),
m_retryCount(0U),
m_type(RCT_NONE),
m_inBuffer(NULL),
m_inLength(0U),
m_outBuffer(NULL),
m_outLength(0U)
{
	assert(!address.empty());
	assert(port > 0U);

	m_address = CUDPReaderWriter::lookup(address);

	m_inBuffer  = new unsigned char[BUFFER_LENGTH];
	m_outBuffer = new unsigned char[BUFFER_LENGTH];
}

CRemoteControlRemoteControlHandler::~CRemoteControlRemoteControlHandler()
{
	delete[] m_inBuffer;
	delete[] m_outBuffer;
}

bool CRemoteControlRemoteControlHandler::open()
{
	return m_socket.open();
}

RC_TYPE CRemoteControlRemoteControlHandler::readType()
{
	m_type = RCT_NONE;

	in_addr address;
	unsigned int port;

	int length = m_socket.read(m_inBuffer, BUFFER_LENGTH, address, port);
	if (length <= 0)
		return m_type;

	m_inLength = length;

	if (::memcmp(m_inBuffer, "ACK", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_ACK;
		return m_type;
	} else if (::memcmp(m_inBuffer, "NAK", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_NAK;
		return m_type;
	} else if (::memcmp(m_inBuffer, "RND", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_RANDOM;
		return m_type;
	} else if (::memcmp(m_inBuffer, "CAL", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_CALLSIGNS;
		return m_type;
	} else if (::memcmp(m_inBuffer, "RPT", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_REPEATER;
		return m_type;
	} else if (::memcmp(m_inBuffer, "SNT", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_STARNET;
		return m_type;
	}

	return m_type;
}

std::string CRemoteControlRemoteControlHandler::readNAK()
{
	if (m_type != RCT_NAK)
		return "";

	std::string text((char*)(m_inBuffer + 3U));

	return text;
}

unsigned int CRemoteControlRemoteControlHandler::readRandom()
{
	if (m_type != RCT_RANDOM)
		return 0U;

	uint32_t random;
	::memcpy(&random, m_inBuffer + 3U, sizeof(uint32_t));

	return CUtils::swap_endian_be(random);
}

CRemoteControlCallsignData* CRemoteControlRemoteControlHandler::readCallsigns()
{
	if (m_type != RCT_CALLSIGNS)
		return NULL;

	CRemoteControlCallsignData* data = new CRemoteControlCallsignData;

	unsigned char* p = m_inBuffer + 3U;
	unsigned int pos = 3U;

	while (pos < m_inLength) {
		unsigned char type = *p;
		pos += 1U;
		p += 1U;

		std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
		pos += LONG_CALLSIGN_LENGTH;
		p += LONG_CALLSIGN_LENGTH;

		switch (type) {
			case 'R':
				data->addRepeater(callsign);
				break;
			case 'S':
				data->addStarNet(callsign);
				break;
			default:		// ????
				break;
		}
	}

	return data;
}

CRemoteControlRepeaterData* CRemoteControlRemoteControlHandler::readRepeater()
{
	if (m_type != RCT_REPEATER)
		return NULL;

	unsigned char* p = m_inBuffer + 3U;
	unsigned int pos = 3U;

	std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	int32_t reconnect;
	::memcpy(&reconnect, p, sizeof(int32_t));
	pos += sizeof(int32_t);
	p += sizeof(int32_t);

	std::string reflector((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	CRemoteControlRepeaterData* data = new CRemoteControlRepeaterData(callsign, CUtils::swap_endian_be(reconnect), reflector);

	while (pos < m_inLength) {
		std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
		pos += LONG_CALLSIGN_LENGTH;
		p += LONG_CALLSIGN_LENGTH;

		int32_t protocol;
		::memcpy(&protocol, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		int32_t linked;
		::memcpy(&linked, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		int32_t direction;
		::memcpy(&direction, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		int32_t dongle;
		::memcpy(&dongle, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		data->addLink(callsign, CUtils::swap_endian_be(protocol), CUtils::swap_endian_be(linked), CUtils::swap_endian_be(direction), CUtils::swap_endian_be(dongle));
	}

	return data;
}

CRemoteControlStarNetGroup* CRemoteControlRemoteControlHandler::readStarNetGroup()
{
	if (m_type != RCT_STARNET)
		return NULL;

	unsigned char* p = m_inBuffer + 3U;
	unsigned int pos = 3U;

	std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	std::string logoff((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	uint32_t timer;
	::memcpy(&timer, p, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	p += sizeof(uint32_t);

	uint32_t timeout;
	::memcpy(&timeout, p, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	p += sizeof(uint32_t);

	CRemoteControlStarNetGroup* group = new CRemoteControlStarNetGroup(callsign, logoff, CUtils::swap_endian_be(timer), CUtils::swap_endian_be(timeout));

	while (pos < m_inLength) {
		std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
		pos += LONG_CALLSIGN_LENGTH;
		p += LONG_CALLSIGN_LENGTH;

		::memcpy(&timer, p, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		p += sizeof(uint32_t);

		::memcpy(&timeout, p, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		p += sizeof(uint32_t);

		group->addUser(callsign, CUtils::swap_endian_be(timer), CUtils::swap_endian_be(timeout));
	}

	return group;
}

bool CRemoteControlRemoteControlHandler::login()
{
	if (m_loggedIn)
		return false;

	if (m_address.s_addr == INADDR_NONE)
		return false;

	::memcpy(m_outBuffer, "LIN", 3U);
	m_outLength = 3U;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

void CRemoteControlRemoteControlHandler::setLoggedIn(bool set)
{
	m_loggedIn = set;
}

bool CRemoteControlRemoteControlHandler::getCallsigns()
{
	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	::memcpy(m_outBuffer, "GCS", 3U);
	m_outLength = 3U;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::sendHash(const unsigned char* hash, unsigned int length)
{
	assert(hash != NULL);
	assert(length > 0U);

	if (m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "SHA", 3U);
	m_outLength += 3U;
	p += 3U;

	::memcpy(p, hash, length);
	m_outLength += length;
	p += length;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::getRepeater(const std::string& callsign)
{
	assert(!callsign.empty());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "GRP", 3U);
	m_outLength += 3U;
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.length(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::getStarNet(const std::string& callsign)
{
	assert(!callsign.empty());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "GSN", 3U);
	m_outLength += 3U;
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.length(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector)
{
	assert(!callsign.empty());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "LNK", 3U);
	m_outLength += 3U;
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.length(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	int32_t temp1 = int32_t(reconnect);
	int32_t temp2 = CUtils::swap_endian_be(temp1);
	::memcpy(p, &temp2, sizeof(int32_t));
	m_outLength += sizeof(int32_t);
	p += sizeof(int32_t);

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < reflector.length(); i++)
		p[i] = reflector.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector)
{
	assert(!callsign.empty());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "UNL", 3U);
	m_outLength += 3U;
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.length(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	int32_t temp1 = int32_t(protocol);
	int32_t temp2 = CUtils::swap_endian_be(temp1);
	::memcpy(p, &temp2, sizeof(int32_t));
	m_outLength += sizeof(int32_t);
	p += sizeof(int32_t);

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < reflector.length(); i++)
		p[i] = reflector.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::logoff(const std::string& callsign, const std::string& user)
{
	assert(!callsign.empty());
	assert(!user.empty());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	::memcpy(p, "LGO", 3U);
	m_outLength += 3U;
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.length(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < user.length(); i++)
		p[i] = user.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CRemoteControlRemoteControlHandler::logout()
{
	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	::memcpy(m_outBuffer, "LOG", 3U);
	m_outLength = 3U;

	for (unsigned int i = 0U; i < 5U; i++) {
		bool ret = m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
		if (!ret) {
			m_retryCount = 0U;
			return false;
		}
	}

	m_retryCount = 1U;

	return true;
}

bool CRemoteControlRemoteControlHandler::retry()
{
	if (m_retryCount > 0U) {
		m_retryCount++;
		if (m_retryCount >= MAX_RETRIES) {
			m_retryCount = 0U;
			return false;
		}

		m_socket.write(m_outBuffer, m_outLength, m_address, m_port);
	}

	return true;
}

void CRemoteControlRemoteControlHandler::close()
{
	m_socket.close();
}
