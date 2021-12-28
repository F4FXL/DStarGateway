/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
 *   copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cstring>
#include <cassert>

#include "RemoteProtocolHandler.h"
#include "DStarDefines.h"
#include "SHA256.h"
#include "Utils.h"
#include "StringUtils.h"

const unsigned int BUFFER_LENGTH = 2000U;

CRemoteProtocolHandler::CRemoteProtocolHandler(unsigned int port, const std::string& address) :
m_socket(address, port),
m_address(),
m_port(0U),
m_loggedIn(false),
m_type(RPHT_NONE),
m_inBuffer(NULL),
m_inLength(0U),
m_outBuffer(NULL)
{
	assert(port > 0U);

	m_inBuffer  = new unsigned char[BUFFER_LENGTH];
	m_outBuffer = new unsigned char[BUFFER_LENGTH];
}

CRemoteProtocolHandler::~CRemoteProtocolHandler()
{
	delete[] m_outBuffer;
	delete[] m_inBuffer;
}

bool CRemoteProtocolHandler::open()
{
	return m_socket.open();
}

RPH_TYPE CRemoteProtocolHandler::readType()
{
	m_type = RPHT_NONE;

	in_addr address;
	unsigned int port;

	int length = m_socket.read(m_inBuffer, BUFFER_LENGTH, address, port);
	if (length <= 0)
		return m_type;

	// CUtils::dump("Incoming", m_inBuffer, length);

	if (::memcmp(m_inBuffer, "LIN", 3U) == 0) {
		m_loggedIn = false;
		m_address  = address;
		m_port     = port;
		m_type = RPHT_LOGIN;
		return m_type;
	}

	if (address.s_addr == inet_addr("127.0.0.1")) {
		if (::memcmp(m_inBuffer, "LKS", 3U) == 0) {
			m_inLength = length;
			m_type = RPHT_LINKSCR;
			return m_type;
		}
	}

	if (m_loggedIn) {
		if (address.s_addr != m_address.s_addr || port != m_port) {
			sendNAK("You are not logged in");
			return m_type;
		}
	}

	m_inLength = length;

	if (::memcmp(m_inBuffer, "SHA", 3U) == 0) {
		if (m_loggedIn) {
			sendNAK("Someone is already logged in");
			return m_type;
		}
		m_type = RPHT_HASH;
		return m_type;
	} else if (::memcmp(m_inBuffer, "GCS", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_CALLSIGNS;
		return m_type;
	} else if (::memcmp(m_inBuffer, "GRP", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_REPEATER;
		return m_type;
	} else if (::memcmp(m_inBuffer, "GSN", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_STARNET;
		return m_type;
	} else if (::memcmp(m_inBuffer, "LNK", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_LINK;
		return m_type;
	} else if (::memcmp(m_inBuffer, "UNL", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_UNLINK;
		return m_type;
	} else if (::memcmp(m_inBuffer, "LGO", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_LOGOFF;
		return m_type;
	} else if (::memcmp(m_inBuffer, "LOG", 3U) == 0) {
		if (!m_loggedIn)
			return m_type;
		m_type = RPHT_LOGOUT;
		return m_type;
	} else {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_UNKNOWN;
		return m_type;
	}
}

bool CRemoteProtocolHandler::readHash(const std::string& password, uint32_t random)
{
	if (m_type != RPHT_HASH)
		return false;

	unsigned char* hash = m_inBuffer + 3U;

	unsigned int len = password.length() + sizeof(uint32_t);
	unsigned char*  in = new unsigned char[len];
	unsigned char* out = new unsigned char[32U];

	::memcpy(in, &random, sizeof(uint32_t));
	for (unsigned int i = 0U; i < password.length(); i++)
		in[i + sizeof(unsigned int)] = password[i];

	CSHA256 sha256;
	sha256.buffer(in, len, out);

	bool res = ::memcmp(out, hash, 32U) == 0;

	delete[] in;
	delete[] out;

	return res;
}

std::string CRemoteProtocolHandler::readRepeater()
{
	if (m_type != RPHT_REPEATER)
		return "";

	std::string callsign((char*)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	return callsign;
}

#ifdef USE_STARNET
std::string CRemoteProtocolHandler::readStarNetGroup()
{
	if (m_type != RPHT_STARNET)
		return "";

	std::string callsign((char*)(m_inBuffer + 3U),  LONG_CALLSIGN_LENGTH);

	return callsign;
}
#endif

bool CRemoteProtocolHandler::readLogoff(std::string& callsign, std::string& user)
{
	if (m_type != RPHT_LOGOFF)
		return false;

	callsign = std::string((char*)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);
	user = std::string((char*)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH), LONG_CALLSIGN_LENGTH);

	return true;
}


bool CRemoteProtocolHandler::readLink(std::string& callsign, RECONNECT& reconnect, std::string& reflector)
{
	if (m_type != RPHT_LINK)
		return false;

	callsign = std::string((char*)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	int32_t temp;
	::memcpy(&temp, m_inBuffer + 3U + LONG_CALLSIGN_LENGTH, sizeof(int32_t));
	reconnect = RECONNECT(CUtils::swap_endian(temp));

	reflector = std::string((char*)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH + sizeof(int32_t)),LONG_CALLSIGN_LENGTH);

	if (reflector == ("        "))
		reflector.clear();

	return true;
}

bool CRemoteProtocolHandler::readUnlink(std::string& callsign, PROTOCOL& protocol, std::string& reflector)
{
	if (m_type != RPHT_UNLINK)
		return false;

	callsign = std::string((char*)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	int32_t temp;
	::memcpy(&temp, m_inBuffer + 3U + LONG_CALLSIGN_LENGTH, sizeof(int32_t));
	protocol = PROTOCOL(CUtils::swap_endian(temp));

	reflector = std::string((char*)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH + sizeof(int32_t)),LONG_CALLSIGN_LENGTH);

	return true;
}

bool CRemoteProtocolHandler::readLinkScr(std::string& callsign, RECONNECT& reconnect, std::string& reflector)
{
	if (m_type != RPHT_LINKSCR)
		return false;

	callsign = std::string((char*)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	reflector = std::string((char*)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH), LONG_CALLSIGN_LENGTH);

	std::string rec = std::string((char*)(m_inBuffer + 3U + 2U * LONG_CALLSIGN_LENGTH),1U);

	unsigned long val = std::stoul(rec);

	reconnect = RECONNECT(val);

	if (reflector == "        ")
		reflector.clear();

	return true;
}

bool CRemoteProtocolHandler::sendCallsigns(const std::vector<std::string> & repeaters, const std::vector<std::string>& starNets)
{
	unsigned char* p = m_outBuffer;

	::memcpy(p, "CAL", 3U);
	p += 3U;

	for (unsigned int n = 0U; n < repeaters.size(); n++) {
		*p++ = 'R';

		::memset(p, ' ' , LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < repeaters[n].length(); i++)
			p[i] = repeaters[n][i];
		p += LONG_CALLSIGN_LENGTH;
	}

	for (unsigned int n = 0U; n < starNets.size(); n++) {
		*p++ = 'S';

		::memset(p, ' ' , LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < starNets[n].length(); i++)
			p[i] = starNets[n][i];
		p += LONG_CALLSIGN_LENGTH;
	}

	// CUtils::dump("Outgoing", m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}

bool CRemoteProtocolHandler::sendRepeater(const CRemoteRepeaterData& data)
{
	unsigned char* p = m_outBuffer;

	::memcpy(p, "RPT", 3U);
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getCallsign().length(); i++)
		p[i] = data.getCallsign()[i];
	p += LONG_CALLSIGN_LENGTH;

	int32_t reconnect = CUtils::swap_endian(data.getReconnect());
	::memcpy(p, &reconnect, sizeof(int32_t));
	p += sizeof(int32_t);

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getReflector().length(); i++)
		p[i] = data.getReflector()[i];
	p += LONG_CALLSIGN_LENGTH;

	for (unsigned int n = 0U; n < data.getLinkCount(); n++) {
		CRemoteLinkData* link = data.getLink(n);

		::memset(p, ' ', LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < link->getCallsign().length(); i++)
			p[i] = link->getCallsign()[i];
		p += LONG_CALLSIGN_LENGTH;

		int32_t protocol = CUtils::swap_endian(link->getProtocol());
		::memcpy(p, &protocol, sizeof(int32_t));
		p += sizeof(int32_t);

		int32_t linked = CUtils::swap_endian(link->isLinked());
		::memcpy(p, &linked, sizeof(int32_t));
		p += sizeof(int32_t);

		int32_t direction = CUtils::swap_endian(link->getDirection());
		::memcpy(p, &direction, sizeof(int32_t));
		p += sizeof(int32_t);

		int32_t dongle = CUtils::swap_endian(link->isDongle());
		::memcpy(p, &dongle, sizeof(int32_t));
		p += sizeof(int32_t);
	}

	// CUtils::dump("Outgoing", m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}

#if USE_STARNET
bool CRemoteProtocolHandler::sendStarNetGroup(const CRemoteStarNetGroup& data)
{
	unsigned char* p = m_outBuffer;

	::memcpy(p, "SNT", 3U);
	p += 3U;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getCallsign().length(); i++)
		p[i] = data.getCallsign().GetChar(i);
	p += LONG_CALLSIGN_LENGTH;

	::memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getLogoff().length(); i++)
		p[i] = data.getLogoff().GetChar(i);
	p += LONG_CALLSIGN_LENGTH;

	uint32_t timer = wxUINT32_SWAP_ON_BE(data.getTimer());
	::memcpy(p, &timer, sizeof(uint32_t));
	p += sizeof(uint32_t);

	uint32_t timeout = wxUINT32_SWAP_ON_BE(data.getTimeout());
	::memcpy(p, &timeout, sizeof(uint32_t));
	p += sizeof(uint32_t);

	for (unsigned int n = 0U; n < data.getUserCount(); n++) {
		CRemoteStarNetUser& user = data.getUser(n);

		::memset(p, ' ', LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < user.getCallsign().length(); i++)
			p[i] = user.getCallsign().GetChar(i);
		p += LONG_CALLSIGN_LENGTH;

		timer = wxUINT32_SWAP_ON_BE(user.getTimer());
		::memcpy(p, &timer, sizeof(uint32_t));
		p += sizeof(uint32_t);

		timeout = wxUINT32_SWAP_ON_BE(user.getTimeout());
		::memcpy(p, &timeout, sizeof(uint32_t));
		p += sizeof(uint32_t);
	}

	// CUtils::dump("Outgoing", m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}
#endif

void CRemoteProtocolHandler::setLoggedIn(bool set)
{
	m_loggedIn = set;
}

void CRemoteProtocolHandler::close()
{
	m_socket.close();
}

bool CRemoteProtocolHandler::sendACK()
{
	::memcpy(m_outBuffer + 0U, "ACK", 3U);

	// CUtils::dump("Outgoing", m_outBuffer, 3U);

	return m_socket.write(m_outBuffer, 3U, m_address, m_port);
}

bool CRemoteProtocolHandler::sendNAK(const std::string& text)
{
	::memcpy(m_outBuffer + 0U, "NAK", 3U);

	::memset(m_outBuffer + 3U, 0x00U, text.length() + 1U);

	for (unsigned int i = 0U; i < text.length(); i++)
		m_outBuffer[i + 3U] = text[i];

	// CUtils::dump("Outgoing", m_outBuffer, 3U + text.length() + 1U);

	return m_socket.write(m_outBuffer, 3U + text.length() + 1U, m_address, m_port);
}

bool CRemoteProtocolHandler::sendRandom(uint32_t random)
{
	::memcpy(m_outBuffer + 0U, "RND", 3U);

	uint32_t temp = CUtils::swap_endian(random);
	::memcpy(m_outBuffer + 3U, &temp, sizeof(uint32_t));

	// CUtils::dump("Outgoing", m_outBuffer, 3U + sizeof(uint32_t));

	return m_socket.write(m_outBuffer, 3U + sizeof(uint32_t), m_address, m_port);
}
