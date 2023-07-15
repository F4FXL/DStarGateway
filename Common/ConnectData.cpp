/*
 *   Copyright (C) 2010,2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>

#include "ConnectData.h"
#include "DStarDefines.h"
#include "Version.h"
#include "Utils.h"
#include "Log.h"
#include "NetUtils.h"

const char *HTML = "<table border=\"0\" width=\"95%%\"><tr><td width=\"4%%\"><img border=\"0\" src=%s></td><td width=\"96%%\"><font size=\"2\"><b>%s</b> DStarGateway %s</font></td></tr></table>";


CConnectData::CConnectData(GATEWAY_TYPE gatewayType, const std::string& repeater, const std::string& reflector, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_gatewayType(gatewayType),
m_repeater(repeater),
m_reflector(reflector),
m_type(type),
m_locator(),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
	assert(repeater.size());
	assert(reflector.size());
}

CConnectData::CConnectData(const std::string& repeater, const std::string& reflector, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(reflector),
m_type(type),
m_locator(),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
	assert(repeater.size());
	assert(reflector.size());
}

CConnectData::CConnectData(const std::string& repeater, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(),
m_type(type),
m_locator(),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
	assert(repeater.size());
}

CConnectData::CConnectData(const std::string& repeater, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(),
m_type(CT_UNLINK),
m_locator(),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
	assert(repeater.size());
}

CConnectData::CConnectData(CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(),
m_reflector(),
m_type(type),
m_locator(),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
}

CConnectData::CConnectData(GATEWAY_TYPE gatewayType, const std::string& repeater, const std::string& reflector, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_gatewayType(gatewayType),
m_repeater(repeater),
m_reflector(reflector),
m_type(type),
m_locator(),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);
	assert(repeater.size());
	assert(reflector.size());

	m_yourAddressAndPort.ss_family = AF_INET6;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
}

CConnectData::CConnectData(const std::string& repeater, const std::string& reflector, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(reflector),
m_type(type),
m_locator(),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);
	assert(repeater.size());
	assert(reflector.size());

	m_yourAddressAndPort.ss_family = AF_INET6;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
}

CConnectData::CConnectData(const std::string& repeater, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(),
m_type(type),
m_locator(),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);
	assert(repeater.size());

	m_yourAddressAndPort.ss_family = AF_INET6;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
}

CConnectData::CConnectData(const std::string& repeater, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(repeater),
m_reflector(),
m_type(CT_UNLINK),
m_locator(),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);
	assert(repeater.size());

	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
}

CConnectData::CConnectData(CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_gatewayType(GT_REPEATER),
m_repeater(),
m_reflector(),
m_type(type),
m_locator(),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);

	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
}

CConnectData::CConnectData() :
m_gatewayType(GT_REPEATER),
m_repeater("        "),
m_reflector(),
m_type(CT_LINK1),
m_locator(),
m_yourAddressAndPort(),
m_myPort(0U)
{
}

CConnectData::~CConnectData()
{
}

bool CConnectData::setDExtraData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 11U);
	assert(yourPort > 0U);

	m_repeater = std::string((const char*)data, LONG_CALLSIGN_LENGTH);
	m_repeater[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH];

	m_reflector.assign("        ");
	m_reflector[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH + 1U];

	switch (length) {
		case 11U:
			if (0 == m_reflector.compare("        "))
				m_type = CT_UNLINK;
			else
				m_type = CT_LINK1;
			break;

		case 14U:
			if (data[LONG_CALLSIGN_LENGTH + 2U] == 'A' &&
				data[LONG_CALLSIGN_LENGTH + 3U] == 'C' &&
				data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
				m_type = CT_ACK;
			else if (data[LONG_CALLSIGN_LENGTH + 2U] == 'N' &&
				     data[LONG_CALLSIGN_LENGTH + 3U] == 'A' &&
					 data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
				m_type = CT_NAK;
			else
				return false;

			break;

		default:
			return false;
	}

	TOIPV4(m_yourAddressAndPort)->sin_addr =  yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
	m_myPort      = myPort;

	return true;
}

bool CConnectData::setDCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 11U);
	assert(yourPort > 0U);

	m_repeater = std::string((const char*)data, LONG_CALLSIGN_LENGTH);
	m_repeater[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH];

	switch (length) {
		case 519U:
			m_reflector = std::string((const char*)(data + LONG_CALLSIGN_LENGTH + 3), LONG_CALLSIGN_LENGTH);
			m_reflector[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH + 1];
			m_type = CT_LINK1;
			break;

		case 19U:
			m_reflector = std::string((const char*)(data + LONG_CALLSIGN_LENGTH + 3U), LONG_CALLSIGN_LENGTH);
			m_reflector[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH + 1];
			m_type = CT_UNLINK;
			break;

		case 14U:
			if (data[LONG_CALLSIGN_LENGTH + 2U] == 'A' &&
				data[LONG_CALLSIGN_LENGTH + 3U] == 'C' &&
				data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
				m_type = CT_ACK;
			else if (data[LONG_CALLSIGN_LENGTH + 2U] == 'N' &&
				     data[LONG_CALLSIGN_LENGTH + 3U] == 'A' &&
					 data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
				m_type = CT_NAK;
			else
				return false;

			break;

		default:
			return false;
	}

	TOIPV4(m_yourAddressAndPort)->sin_addr =  yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
	m_myPort      = myPort;

	return true;
}

bool CConnectData::setCCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 14U);
	assert(yourPort > 0U);

	m_repeater = std::string((const char*)data, LONG_CALLSIGN_LENGTH);
	m_repeater[LONG_CALLSIGN_LENGTH - 1] = data[LONG_CALLSIGN_LENGTH];

	if (data[LONG_CALLSIGN_LENGTH + 2U] == 'A' &&
		data[LONG_CALLSIGN_LENGTH + 3U] == 'C' &&
		data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
		m_type = CT_ACK;
	else if (data[LONG_CALLSIGN_LENGTH + 2U] == 'N' &&
		     data[LONG_CALLSIGN_LENGTH + 3U] == 'A' &&
			 data[LONG_CALLSIGN_LENGTH + 4U] == 'K')
		m_type = CT_NAK;
	else
		return false;

	TOIPV4(m_yourAddressAndPort)->sin_addr =  yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
	m_myPort      = myPort;

	return true;
}

bool CConnectData::setDPlusData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 5U);
	assert(yourPort > 0U);

	switch (length) {
		case 5U:
			switch (data[4U]) {
				case 0x01:
					m_type = CT_LINK1;
					break;
				case 0x00:
					m_type = CT_UNLINK;
					break;
			}
			break;

		case 8U: {
				std::string reply((const char*)(data + 4U), 4U);
				CLog::logInfo("D-Plus reply is %.4s\n", reply.c_str());

				if (::memcmp(data + 4U, "OKRW", 4U) == 0)
					m_type = CT_ACK;
				else
					m_type = CT_NAK;
			}
			break;

		case 28U:
			m_repeater = std::string((const char*)(data + 4U), LONG_CALLSIGN_LENGTH);
			m_type = CT_LINK2;
			break;

		default:
			return false;
	}

	TOIPV4(m_yourAddressAndPort)->sin_addr =  yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)yourPort);
	m_myPort      = myPort;

	return true;
}

unsigned int CConnectData::getDExtraData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 11U);

	::memset(data, ' ', LONG_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < m_repeater.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
		data[i] = m_repeater.at(i);

	data[LONG_CALLSIGN_LENGTH] = m_repeater.at(LONG_CALLSIGN_LENGTH - 1U);

	switch (m_type) {
		case CT_LINK1:
		case CT_LINK2:
			data[LONG_CALLSIGN_LENGTH + 1U] = m_reflector.at(LONG_CALLSIGN_LENGTH - 1U);
			data[LONG_CALLSIGN_LENGTH + 2U] = 0x00;
			return 11U;

		case CT_UNLINK:
			data[LONG_CALLSIGN_LENGTH + 1U] = ' ';
			data[LONG_CALLSIGN_LENGTH + 2U] = 0x00;
			return 11U;

		case CT_ACK:
			data[LONG_CALLSIGN_LENGTH + 1U] = m_reflector.at(LONG_CALLSIGN_LENGTH - 1U);
			data[LONG_CALLSIGN_LENGTH + 2U] = 'A';
			data[LONG_CALLSIGN_LENGTH + 3U] = 'C';
			data[LONG_CALLSIGN_LENGTH + 4U] = 'K';
			data[LONG_CALLSIGN_LENGTH + 5U] = 0x00;
			return 14U;

		case CT_NAK:
			data[LONG_CALLSIGN_LENGTH + 1U] = m_reflector.at(LONG_CALLSIGN_LENGTH - 1U);
			data[LONG_CALLSIGN_LENGTH + 2U] = 'N';
			data[LONG_CALLSIGN_LENGTH + 3U] = 'A';
			data[LONG_CALLSIGN_LENGTH + 4U] = 'K';
			data[LONG_CALLSIGN_LENGTH + 5U] = 0x00;
			return 14U;

		default:
			return 0U;
	}
}

unsigned int CConnectData::getDCSData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 519U);

	::memset(data, ' ', LONG_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < m_repeater.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
		data[i] = m_repeater.at(i);

	data[LONG_CALLSIGN_LENGTH] = m_repeater.at(LONG_CALLSIGN_LENGTH - 1U);

	switch (m_type) {
		case CT_LINK1:
		case CT_LINK2: {
				data[LONG_CALLSIGN_LENGTH + 1U] = m_reflector.at(LONG_CALLSIGN_LENGTH - 1U);
				data[LONG_CALLSIGN_LENGTH + 2U] = 0x00U;
				::memset(data + 11U, ' ', LONG_CALLSIGN_LENGTH);
				for (unsigned int i = 0U; i < m_reflector.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
					data[i + 11U] = m_reflector.at(i);

				char shtml[512];
				switch (m_gatewayType) {
					case GT_HOTSPOT:
						snprintf(shtml, 512, HTML, "hotspot.jpg", "HOTSPOT", VERSION.c_str());
						break;
					case GT_DONGLE:
						snprintf(shtml, 512, HTML, "dongle.jpg", "DONGLE", VERSION.c_str());
						break;
					case GT_SMARTGROUP:
						snprintf(shtml, 512, HTML, "hf.jpg", "Smart Group", VERSION.c_str());
						break;
					default:
						snprintf(shtml, 512, HTML, "hf.jpg", "REPEATER", VERSION.c_str());
						break;
				}
				std::string html(shtml);

				::memset(data + 19U, 0x00U, 500U);
				for (unsigned int i = 0U; i < html.size(); i++)
					data[i + 19U] = html.at(i);
			}
			return 519U;

		case CT_UNLINK:
			data[LONG_CALLSIGN_LENGTH + 1U] = 0x20U;
			data[LONG_CALLSIGN_LENGTH + 2U] = 0x00U;
			::memset(data + 11U, ' ', LONG_CALLSIGN_LENGTH);
			for (unsigned int i = 0U; i < m_reflector.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
				data[i + 11U] = m_reflector.at(i);
			return 19U;

		case CT_ACK:
			data[LONG_CALLSIGN_LENGTH + 1U] = m_reflector.at(LONG_CALLSIGN_LENGTH - 1U);
			data[LONG_CALLSIGN_LENGTH + 2U] = 'A';
			data[LONG_CALLSIGN_LENGTH + 3U] = 'C';
			data[LONG_CALLSIGN_LENGTH + 4U] = 'K';
			data[LONG_CALLSIGN_LENGTH + 5U] = 0x00;
			return 14U;

		case CT_NAK:
			data[LONG_CALLSIGN_LENGTH + 1U] = 0x20U;
			data[LONG_CALLSIGN_LENGTH + 2U] = 'N';
			data[LONG_CALLSIGN_LENGTH + 3U] = 'A';
			data[LONG_CALLSIGN_LENGTH + 4U] = 'K';
			data[LONG_CALLSIGN_LENGTH + 5U] = 0x00;
			return 14U;

		default:
			return 0U;
	}
}

unsigned int CConnectData::getCCSData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 39U);

	::memset(data, ' ', 39U);

	for (unsigned int i = 0U; i < m_repeater.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
		data[i] = m_repeater.at(i);

	data[LONG_CALLSIGN_LENGTH + 0U] = m_repeater.at(LONG_CALLSIGN_LENGTH - 1U);

	switch (m_type) {
		case CT_LINK1:
		case CT_LINK2: {
				data[9U]  = 0x41U;
				data[10U] = '@';

				for (unsigned int i = 0U; i < m_locator.size(); i++)
					data[11U + i] = m_locator.at(i);

				data[17U] = 0x20U;
				data[18U] = '@';

				std::string text("ircDDB_GW-");
				text += VERSION.substr(0, 8);

				for (unsigned int i = 0U; i < text.size(); i++)
					data[19U + i] = text.at(i);
			}
			return 39U;

		case CT_UNLINK:
			return 19U;

		default:
			return 0U;
	}
}

unsigned int CConnectData::getDPlusData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 28U);

	switch (m_type) {
		case CT_LINK1:
			data[0U] = 0x05;
			data[1U] = 0x00;
			data[2U] = 0x18;
			data[3U] = 0x00;
			data[4U] = 0x01;
			return 5U;

		case CT_LINK2: {
				data[0U]  = 0x1C;
				data[1U]  = 0xC0;
				data[2U]  = 0x04;
				data[3U]  = 0x00;

				for (unsigned int i = 4U; i < 20U; i++)
					data[i] = 0x00;

				std::string callsign = m_repeater;
				CUtils::Trim(callsign);

				for (unsigned int i = 0U; i < callsign.size(); i++)
					data[i + 4U] = callsign.at(i);

				data[20U] = 'D';
				data[21U] = 'V';
				data[22U] = '0';
				data[23U] = '1';
				data[24U] = '9';
				data[25U] = '9';
				data[26U] = '9';
				data[27U] = '9';
			}
			return 28U;

		case CT_UNLINK:
			data[0U] = 0x05;
			data[1U] = 0x00;
			data[2U] = 0x18;
			data[3U] = 0x00;
			data[4U] = 0x00;
			return 5U;

		case CT_ACK:
			data[0U] = 0x08;
			data[1U] = 0xC0;
			data[2U] = 0x04;
			data[3U] = 0x00;
			data[4U] = 'O';
			data[5U] = 'K';
			data[6U] = 'R';
			data[7U] = 'W';
			return 8U;

		case CT_NAK:
			data[0U] = 0x08;
			data[1U] = 0xC0;
			data[2U] = 0x04;
			data[3U] = 0x00;
			data[4U] = 'B';
			data[5U] = 'U';
			data[6U] = 'S';
			data[7U] = 'Y';
			return 8U;

		default:
			return 0U;
	}
}

unsigned int CConnectData::getMyPort() const
{
	return m_myPort;
}

std::string CConnectData::getRepeater() const
{
	return m_repeater;
}

std::string CConnectData::getReflector() const
{
	return m_reflector;
}

CD_TYPE CConnectData::getType() const
{
	return m_type;
}

void CConnectData::setLocator(const std::string& locator)
{
	m_locator = locator;
}
