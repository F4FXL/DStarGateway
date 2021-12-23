/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017,2018 by Thomas A. Early N7TAE
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
#include <cstring>

#include "RemoteProtocolHandler.h"
#include "DStarDefines.h"
#include "SHA256.h"
#include "Utils.h"

#define wxINT32_SWAP_ON_BE(x) x
#define wxUINT32_SWAP_ON_BE(x) x

const unsigned int BUFFER_LENGTH = 2000U;

CRemoteProtocolHandler::CRemoteProtocolHandler(unsigned int port, const std::string &address) :
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

	if (memcmp(m_inBuffer, "LIN", 3U) == 0) {
		m_loggedIn = false;
		m_address  = address;
		m_port     = port;
		m_type = RPHT_LOGIN;
		return m_type;
	}

	if (address.s_addr == inet_addr("127.0.0.1")) {
		if (memcmp(m_inBuffer, "LKS", 3U) == 0) {
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

	if (memcmp(m_inBuffer, "SHA", 3U) == 0) {
		if (m_loggedIn) {
			sendNAK("Someone is already logged in");
			return m_type;
		}
		m_type = RPHT_HASH;
		return m_type;
	} else if (memcmp(m_inBuffer, "GCS", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_CALLSIGNS;
		return m_type;
	} else if (memcmp(m_inBuffer, "GRP", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_REPEATER;
		return m_type;
	} else if (memcmp(m_inBuffer, "GSN", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_SMARTGROUP;
		return m_type;
	} else if (memcmp(m_inBuffer, "LNK", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_LINK;
		return m_type;
	} else if (memcmp(m_inBuffer, "UNL", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_UNLINK;
		return m_type;
	} else if (memcmp(m_inBuffer, "LGO", 3U) == 0) {
		if (!m_loggedIn) {
			sendNAK("You are not logged in");
			return m_type;
		}
		m_type = RPHT_LOGOFF;
		return m_type;
	} else if (memcmp(m_inBuffer, "LOG", 3U) == 0) {
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

bool CRemoteProtocolHandler::readHash(const std::string &password, uint32_t random)
{
	if (m_type != RPHT_HASH)
		return false;

	unsigned char *hash = m_inBuffer + 3U;

	unsigned int len = password.size() + sizeof(uint32_t);
	unsigned char *in = new unsigned char[len];
	unsigned char *out = new unsigned char[32U];

	memcpy(in, &random, sizeof(uint32_t));
	for (unsigned int i = 0U; i < password.size(); i++)
		in[i + sizeof(unsigned int)] = password.at(i);

	CSHA256 sha256;
	sha256.buffer(in, len, out);

	bool res = memcmp(out, hash, 32U) == 0;

	delete[] in;
	delete[] out;

	return res;
}

std::string CRemoteProtocolHandler::readRepeater()
{
	if (m_type != RPHT_REPEATER)
		return std::string("");

	std::string callsign((char *)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	return callsign;
}

std::string CRemoteProtocolHandler::readGroup()
{
	if (m_type != RPHT_SMARTGROUP)
		return std::string("");

	std::string callsign((char *)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	return callsign;
}

bool CRemoteProtocolHandler::readLogoff(std::string &callsign, std::string &user)
{
	if (m_type != RPHT_LOGOFF)
		return false;

	callsign = std::string((char *)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);
	user = std::string((char *)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH), LONG_CALLSIGN_LENGTH);

	return true;
}


bool CRemoteProtocolHandler::readLink(std::string &callsign, std::string &reflector)
{
	if (m_type != RPHT_LINK)
		return false;

	callsign = std::string((char *)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	reflector = std::string((char *)(m_inBuffer + 3U + LONG_CALLSIGN_LENGTH), LONG_CALLSIGN_LENGTH);

	if (0==reflector.compare("        "))
		return false;

	return true;
}

bool CRemoteProtocolHandler::readUnlink(std::string &callsign)
{
	if (m_type != RPHT_UNLINK)
		return false;

	callsign = std::string((char *)(m_inBuffer + 3U), LONG_CALLSIGN_LENGTH);

	return true;
}

bool CRemoteProtocolHandler::sendCallsigns(const std::list<std::string> &repeaters, const std::list<std::string> &groups)
{
	unsigned char *p = m_outBuffer;

	memcpy(p, "CAL", 3U);
	p += 3U;

	for (auto it=repeaters.cbegin(); it!=repeaters.cend(); it++) {
		*p++ = 'R';

		memset(p, ' ' , LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < it->size(); i++)
			p[i] = it->at(i);
		p += LONG_CALLSIGN_LENGTH;
	}

	for (auto it=groups.cbegin(); it!=groups.cend(); it++) {
		*p++ = 'S';

		memset(p, ' ' , LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < it->size(); i++)
			p[i] = it->at(i);
		p += LONG_CALLSIGN_LENGTH;
	}

	// CUtils::dump(wxT("Outgoing"), m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}

bool CRemoteProtocolHandler::sendRepeater(const CRemoteRepeaterData &data)
{
	unsigned char *p = m_outBuffer;

	memcpy(p, "RPT", 3U);
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getCallsign().size(); i++)
		p[i] = data.getCallsign().at(i);
	p += LONG_CALLSIGN_LENGTH;

	uint32_t reconnect = wxINT32_SWAP_ON_BE(data.getReconnect());
	memcpy(p, &reconnect, sizeof(uint32_t));
	p += sizeof(uint32_t);

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getReflector().size(); i++)
		p[i] = data.getReflector().at(i);
	p += LONG_CALLSIGN_LENGTH;

	for (unsigned int n = 0U; n < data.getLinkCount(); n++) {
		CRemoteLinkData *link = data.getLink(n);

		memset(p, ' ', LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < link->getCallsign().size(); i++)
			p[i] = link->getCallsign().at(i);
		p += LONG_CALLSIGN_LENGTH;

		uint32_t protocol = wxINT32_SWAP_ON_BE(link->getProtocol());
		memcpy(p, &protocol, sizeof(uint32_t));
		p += sizeof(uint32_t);

		uint32_t linked = wxINT32_SWAP_ON_BE(link->isLinked());
		memcpy(p, &linked, sizeof(uint32_t));
		p += sizeof(uint32_t);

		uint32_t direction = wxINT32_SWAP_ON_BE(link->getDirection());
		memcpy(p, &direction, sizeof(uint32_t));
		p += sizeof(uint32_t);

		uint32_t dongle = wxINT32_SWAP_ON_BE(link->isDongle());
		memcpy(p, &dongle, sizeof(uint32_t));
		p += sizeof(uint32_t);
	}

	// CUtils::dump("Outgoing", m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}

bool CRemoteProtocolHandler::sendGroup(const CRemoteGroup &data)
{
	unsigned char *p = m_outBuffer;

	memcpy(p, "SNT", 3U);
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getCallsign().size(); i++)
		p[i] = data.getCallsign().at(i);
	p += LONG_CALLSIGN_LENGTH;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getLogoff().size(); i++)
		p[i] = data.getLogoff().at(i);
	p += LONG_CALLSIGN_LENGTH;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getRepeater().size(); i++)
		p[i] = data.getRepeater().at(i);
	p += LONG_CALLSIGN_LENGTH;

	memset(p, ' ', 20);
	for (unsigned int i = 0U; i < data.getInfoText().size(); i++)
		p[i] = data.getInfoText().at(i);
	p += 20;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < data.getReflector().size(); i++)
		p[i] = data.getReflector().at(i);
	p += LONG_CALLSIGN_LENGTH;

	LINK_STATUS ls = data.getLinkStatus();
	memcpy(p, &ls, sizeof(enum LINK_STATUS));
	p += sizeof(enum LINK_STATUS);

	unsigned int ut = data.getUserTimeout();
	memcpy(p, &ut, sizeof(unsigned int));
	p += sizeof(unsigned int);

	for (unsigned int n = 0U; n < data.getUserCount(); n++) {
		CRemoteUser *user = data.getUser(n);

		memset(p, ' ', LONG_CALLSIGN_LENGTH);
		for (unsigned int i = 0U; i < user->getCallsign().size(); i++)
			p[i] = user->getCallsign().at(i);
		p += LONG_CALLSIGN_LENGTH;

		uint32_t timer = wxUINT32_SWAP_ON_BE(user->getTimer());
		memcpy(p, &timer, sizeof(uint32_t));
		p += sizeof(uint32_t);

		uint32_t timeout = wxUINT32_SWAP_ON_BE(user->getTimeout());
		memcpy(p, &timeout, sizeof(uint32_t));
		p += sizeof(uint32_t);
	}

	// CUtils::dump("Outgoing", m_outBuffer, p - m_outBuffer);

	return m_socket.write(m_outBuffer, p - m_outBuffer, m_address, m_port);
}

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
	memcpy(m_outBuffer + 0U, "ACK", 3U);

	// CUtils::dump("Outgoing", m_outBuffer, 3U);

	return m_socket.write(m_outBuffer, 3U, m_address, m_port);
}

bool CRemoteProtocolHandler::sendNAK(const std::string &text)
{
	memcpy(m_outBuffer + 0U, "NAK", 3U);

	memset(m_outBuffer + 3U, 0x00U, text.size() + 1U);

	for (unsigned int i = 0U; i < text.size(); i++)
		m_outBuffer[i + 3U] = text.at(i);

	// CUtils::dump("Outgoing", m_outBuffer, 3U + text.size() + 1U);

	return m_socket.write(m_outBuffer, 3U + text.size() + 1U, m_address, m_port);
}

bool CRemoteProtocolHandler::sendRandom(uint32_t random)
{
	memcpy(m_outBuffer + 0U, "RND", 3U);

	uint32_t temp = wxUINT32_SWAP_ON_BE(random);
	memcpy(m_outBuffer + 3U, &temp, sizeof(uint32_t));

	// CUtils::dump("Outgoing", m_outBuffer, 3U + sizeof(uint32_t));

	return m_socket.write(m_outBuffer, 3U + sizeof(uint32_t), m_address, m_port);
}
