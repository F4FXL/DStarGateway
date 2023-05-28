/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
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

#include "HeardData.h"

CHeardData::CHeardData() :
m_reflector(),
m_repeater(),
m_user(),
m_ext(),
m_address(),
m_port(0U)
{
}

CHeardData::CHeardData(const CHeardData& data) :
m_reflector(data.m_reflector),
m_repeater(data.m_repeater),
m_user(data.m_user),
m_ext(data.m_ext),
m_address(data.m_address),
m_port(data.m_port)
{
}

CHeardData::CHeardData(const CHeaderData& data, const std::string& repeater, const std::string& reflector) :
m_reflector(reflector),
m_repeater(repeater),
m_user(),
m_ext(),
m_address(),
m_port()
{
	m_user = data.getMyCall1();
	m_ext  = data.getMyCall2();
}

CHeardData::~CHeardData()
{
}

bool CHeardData::setIcomRepeaterData(const unsigned char *data, unsigned int length, const in_addr& address, unsigned int port)
{
	assert(data != NULL);
	assert(length >= 26U);

	std::string suser((const char *)data + 10U);
	std::string srptr((const char *)data + 18U);

	m_user     = suser.substr(0, LONG_CALLSIGN_LENGTH);
	m_repeater = srptr.substr(0, LONG_CALLSIGN_LENGTH);

	m_address = address;
	m_port    = port;

	return true;
}

unsigned int CHeardData::getCCSData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 100U);

	::memset(data, 0x00U, 100U);

	data[0U] = '0';
	data[1U] = '0';
	data[2U] = '0';
	data[3U] = '1';

	::memset(data + 7U, ' ', 36U);

	for (unsigned int i = 0U; i < m_reflector.size(); i++)
		data[i + 7U] = m_reflector.at(i);

	for (unsigned int i = 0U; i < m_repeater.size(); i++)
		data[i + 15U] = m_repeater.at(i);

	::memcpy(data + 23U, "CQCQCQ  ",  LONG_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < m_user.size(); i++)
		data[i + 31U] = m_user.at(i);

	for (unsigned int i = 0U; i < m_ext.size(); i++)
		data[i + 39U] = m_ext.at(i);

	data[61U] = 0x01U;

	data[63U] = 0x21U;

	::memset(data + 64U, ' ', 20U);

	data[93U] = 0x36U;

	return 100U;
}

std::string CHeardData::getRepeater() const
{
	return m_repeater;
}

std::string CHeardData::getUser() const
{
	return m_user;
}

void CHeardData::setDestination(const in_addr& address, unsigned int port)
{
	m_address = address;
	m_port    = port;
}

in_addr CHeardData::getAddress() const
{
	return m_address;
}

unsigned int CHeardData::getPort() const
{
	return m_port;
}
