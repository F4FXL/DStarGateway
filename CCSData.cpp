/*
 *   Copyright (C) 2013 by Jonathan Naylor G4KLX
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

#include <cassert>
#include <cstring>

#include "DStarDefines.h"
#include "CCSData.h"
#include "Utils.h"

CCCSData::CCCSData(const std::string& local, double latitude, double longitude, double frequency, double offset, const std::string& description1, const std::string& description2, const std::string& url, CC_TYPE type) :
m_local(local),
m_remote(),
m_latitude(latitude),
m_longitude(longitude),
m_frequency(frequency),
m_offset(offset),
m_description1(description1),
m_description2(description2),
m_url(url),
m_type(type),
m_yourAddress(),
m_yourPort(0U),
m_myPort(0U)
{
}

CCCSData::CCCSData(const std::string& local, const std::string& remote, CC_TYPE type) :
m_local(local),
m_remote(remote),
m_latitude(0.0),
m_longitude(0.0),
m_frequency(0.0),
m_offset(0.0),
m_description1(),
m_description2(),
m_url(),
m_type(type),
m_yourAddress(),
m_yourPort(0U),
m_myPort(0U)
{
}

CCCSData::CCCSData() :
m_local(),
m_remote(),
m_latitude(0.0),
m_longitude(0.0),
m_frequency(0.0),
m_offset(0.0),
m_description1(),
m_description2(),
m_url(),
m_type(),
m_yourAddress(),
m_yourPort(0U),
m_myPort(0U)
{
}

CCCSData::~CCCSData()
{
}

bool CCCSData::setCCSData(const unsigned char *data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	char buffer[LONG_CALLSIGN_LENGTH+1];
	switch (length) {
		case 100U:
			memcpy(buffer, data, LONG_CALLSIGN_LENGTH);
			buffer[LONG_CALLSIGN_LENGTH] = (char)0;
			m_remote = std::string(buffer);

			if (0 == memcmp(data + 8U, "0001", 4U)) {
				m_type = CT_TERMINATE;
			} else {
				// CUtils::dump(wxT("Invalid CCS packet"), data, length);
				return false;
			}
			memcpy(buffer, data+12, LONG_CALLSIGN_LENGTH);
			buffer[LONG_CALLSIGN_LENGTH] = (char)0;
			m_local = std::string(buffer);
			break;

		case 20U:
			if (0 == memcmp(data + 0U, "DTMF_CALL:", 10U)) {
				m_type = CT_DTMFFOUND;
			} else {
				CUtils::dump("Invalid CCS packet", data, length);
				return false;
			}
			memcpy(buffer, data+10, LONG_CALLSIGN_LENGTH);
			buffer[LONG_CALLSIGN_LENGTH] = (char)0;
			m_remote = std::string(buffer);
			break;

		case 17U:
			if (0 == memcmp(data + 0U, "NODTMFCALL", 10U)) {
				m_type = CT_DTMFNOTFOUND;
			} else {
				CUtils::dump("Invalid CCS packet", data, length);
				return false;
			}
			break;

		default:
			CUtils::dump("Invalid CCS packet", data, length);
			return false;
	}

	m_yourAddress = yourAddress;
	m_yourPort    = yourPort;
	m_myPort      = myPort;

	return true;
}

unsigned int CCCSData::getCCSData(unsigned char* data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 133U);

	if (m_type == CT_TERMINATE) {
		memset(data, ' ', 38U);

		for (unsigned int i = 0U; i < m_remote.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i] = m_remote.at(i);

		memcpy(data + 8U, "0001", 4U);

		for (unsigned int i = 0U; i < m_local.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i + 12U] = m_local.at(i);

		return 38U;
	} else if (m_type == CT_INFO) {
		char bstr[256];
		snprintf(bstr, 256, "IRPT%.7s %s%-10.4lf%-10.4lf%-10.4lf%-10.4lf%-20s%-20s%-40s", m_local.substr(0U, LONG_CALLSIGN_LENGTH - 1U).c_str(), m_local.substr(LONG_CALLSIGN_LENGTH - 1U, 1U).c_str(), m_latitude, m_longitude, m_frequency, m_offset, m_description1.c_str(), m_description2.c_str(), m_url.c_str());
		unsigned int len = strlen(bstr);
		if (len > 133U)
			len = 133u;
		memcpy(data, bstr, len);

		return 133U;
	}

	return 0U;
}

std::string CCCSData::getLocal() const
{
	return m_local;
}

std::string CCCSData::getRemote() const
{
	return m_remote;
}

CC_TYPE CCCSData::getType() const
{
	return m_type;
}

void CCCSData::setDestination(const in_addr& address, unsigned int port)
{
	m_yourAddress = address;
	m_yourPort    = port;
}

in_addr CCCSData::getYourAddress() const
{
	return m_yourAddress;
}

unsigned int CCCSData::getYourPort() const
{
	return m_yourPort;
}

unsigned int CCCSData::getMyPort() const
{
	return m_myPort;
}
