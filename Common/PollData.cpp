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

#include <cassert>
#include <cstring>

#include "PollData.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "NetUtils.h"

CPollData::CPollData(const std::string& data1, const std::string& data2, DIRECTION direction, const sockaddr_storage& yourAddressAndPort, unsigned int myPort) :
m_data1(data1),
m_data2(data2),
m_direction(direction),
m_dongle(false),
m_length(0U),
m_yourAddressAndPort(yourAddressAndPort),
m_myPort(myPort)
{
	assert(GETPORT(yourAddressAndPort) > 0U);
}

CPollData::CPollData(const std::string& data1, const std::string& data2, DIRECTION direction, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_data1(data1),
m_data2(data2),
m_direction(direction),
m_dongle(false),
m_length(0U),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);

	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
}

CPollData::CPollData(const std::string& data, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_data1(data),
m_data2(),
m_direction(DIR_OUTGOING),
m_dongle(false),
m_length(0U),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);

	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
}

CPollData::CPollData(const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort) :
m_data1(),
m_data2(),
m_direction(DIR_OUTGOING),
m_dongle(false),
m_length(0U),
m_yourAddressAndPort(),
m_myPort(myPort)
{
	assert(yourPort > 0U);

	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
}

CPollData::CPollData() :
m_data1(),
m_data2(),
m_direction(DIR_OUTGOING),
m_dongle(false),
m_length(0U),
m_yourAddressAndPort(),
m_myPort(0U)
{
}

CPollData::~CPollData()
{
}

bool CPollData::setDExtraData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 9U);
	assert(yourPort > 0U);

	m_data1   = std::string((const char*)data);
	m_data1.resize(LONG_CALLSIGN_LENGTH, ' ');
	m_dongle  = data[LONG_CALLSIGN_LENGTH] != 0x00;

	m_length      = length;
	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
	m_myPort      = myPort;

	return true;
}

bool CPollData::setDCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(yourPort > 0U);

	std::string sdata((const char *)data);

	switch (length) {
		case 17U:
			m_data1       = sdata.substr(0, LONG_CALLSIGN_LENGTH);
			m_data2       = sdata.substr(9, LONG_CALLSIGN_LENGTH);
			m_length      = length;
			m_direction   = DIR_INCOMING;
			m_yourAddressAndPort.ss_family = AF_INET;
			TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
			SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
			m_myPort      = myPort;
			break;

		case 22U:
			m_data1       = sdata.substr(0, LONG_CALLSIGN_LENGTH);
			m_data2       = sdata.substr(9, LONG_CALLSIGN_LENGTH - 1U);
			m_data2.push_back(sdata[17]);
			m_length      = length;
			m_direction   = DIR_OUTGOING;
			m_yourAddressAndPort.ss_family = AF_INET;
			TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
			SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
			m_myPort      = myPort;
			break;
	}

	return true;
}

bool CPollData::setCCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(data != NULL);
	assert(length >= 25U);
	assert(yourPort > 0U);

	m_data1       = std::string((const char*)data);
	m_data1.resize(25);
	m_length      = length;
	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
	m_myPort      = myPort;

	return true;
}

bool CPollData::setDPlusData(const unsigned char* /*data*/, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort)
{
	assert(yourPort > 0U);

	m_length      = length;
	m_yourAddressAndPort.ss_family = AF_INET;
	TOIPV4(m_yourAddressAndPort)->sin_addr = yourAddress;
	SETPORT(m_yourAddressAndPort, (in_port_t)myPort);
	m_myPort      = myPort;

	return true;
}

unsigned int CPollData::getDExtraData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 9U);

	::memset(data, ' ', LONG_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < m_data1.size() && i < LONG_CALLSIGN_LENGTH; i++)
		data[i] = m_data1.at(i);

	data[LONG_CALLSIGN_LENGTH] = 0x00;

	return 9U;
}

unsigned int CPollData::getDCSData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 22U);

	if (m_direction == DIR_OUTGOING) {
		::memset(data, ' ', 17U);

		for (unsigned int i = 0U; i < m_data1.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i + 0U] = m_data1.at(i);

		data[8U] = 0x00U;

		for (unsigned int i = 0U; i < m_data2.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i + 9U] = m_data2.at(i);

		return 17U;
	} else {
		::memset(data, ' ', 22U);

		for (unsigned int i = 0U; i < m_data1.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i + 0U] = m_data1.at(i);

		for (unsigned int i = 0U; i < m_data2.size() && i < (LONG_CALLSIGN_LENGTH - 1U); i++)
			data[i + 9U] = m_data2.at(i);

		if (m_data2.size() >= LONG_CALLSIGN_LENGTH)
			data[17U] = m_data2.at(LONG_CALLSIGN_LENGTH - 1U);

		data[18U] = 0x0AU;
		data[19U] = 0x00U;

		return 22U;
	}
}

unsigned int CPollData::getCCSData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 25U);

	::memset(data, ' ', 25U);

	for (unsigned int i = 0U; i < m_data1.size() && i < LONG_CALLSIGN_LENGTH; i++)
		data[i + 0U] = m_data1.at(i);

	if (m_data2.size()) {
		for (unsigned int i = 0U; i < m_data2.size() && i < LONG_CALLSIGN_LENGTH; i++)
			data[i + 8U] = m_data2.at(i);
	}

	return 25U;
}

unsigned int CPollData::getDPlusData(unsigned char *data, unsigned int length) const
{
	assert(data != NULL);
	assert(length >= 3U);

	data[0U] = 0x03;
	data[1U] = 0x60;
	data[2U] = 0x00;

	return 3U;
}

std::string CPollData::getData1() const
{
	return m_data1;
}

void CPollData::setData1(const std::string& data)
{
	m_data1 = data;
}

std::string CPollData::getData2() const
{
	return m_data2;
}

void CPollData::setData2(const std::string& data)
{
	m_data2 = data;
}

bool CPollData::isDongle() const
{
	return m_dongle;
}

sockaddr_storage CPollData::getYourAddressAndPort() const
{
	return m_yourAddressAndPort;
}

unsigned int CPollData::getMyPort() const
{
	return m_myPort;
}

DIRECTION CPollData::getDirection() const
{
	return m_direction;
}

unsigned int CPollData::getLength() const
{
	return m_length;
}
